#include "Connection.h"
#include "../../util/src/Logger.h"

using std::string;
using std::vector;
using std::deque;
using std::lock_guard;
using std::mutex;
using std::cout;

namespace asio = boost::asio;
using boost::asio::buffer;

Connection::Connection(asio::io_service& ioService,
                       string ip,
                       int port,
                       string connectionId)
    :socket_(ioService),
     receivedMessages_(),
     sentMessages_(),
     connectionId_(connectionId),
     messageCount_(0),
     mutex_(),
     closed_(false)
{
    using boost::asio::ip::address;
    tcp::endpoint endPoint(address::from_string(ip), port);
    socket_.connect(endPoint);
    
    prepareMessage();
}

Connection::Connection(asio::io_service& ioService,
                       tcp::socket socket,
                       string connectionId)
    :socket_(std::move(socket)),
     receivedMessages_(),
     sentMessages_(),
     connectionId_(connectionId),
     messageCount_(0),
     mutex_(),
     closed_(true)
{
    closed_ = !socket_.is_open();
    if (!closed_) {
        prepareMessage();
    }
}

void Connection::prepareMessage()
{
    //create message
    //for the first client
    Message message(getMessageId());
    receivedMessages_.push_back(std::move(message));
    //create wrapper for member function
    auto onMessageReceivedWrapper = [this](const error_code& ec,
                                           size_t nrBytes)
    {
        this->onMessageReceived(ec, nrBytes);
    };
    //start async read operation
    socket_.async_read_some(buffer(receivedMessages_.back().buffer),
                            onMessageReceivedWrapper);
}
void Connection::onMessageReceived(const error_code& ec,
                                   size_t nrBytes)
{
    LOG << INFO << Logger::trace 
        << " Connection::onMessageReceived "
        << connectionId_
        << "\n";
        
    if (!ec)
    {
        //sync access for the received messages queue,
        //because it can be accessed by concurrent threads:
        //the main thread calling receive messages and the
        //other threads executing this callback function
        lock_guard<mutex> lock(mutex_);

        Message& message = receivedMessages_.back();
        //resize the vetor to keep only
        //the date received from the network
        message.buffer.resize(nrBytes);
        message.complete = true;

        //prepare for the next message
        //to be received
        prepareMessage();
    }
    else
    {
        //mark the connection as closed
        {//enter critical section
            lock_guard<mutex> lock(mutex_);
            closed_ = true;
        }//exit critical secation
    }
}

void Connection::sendMessage(Message& message)
{
    if (closed_)
    {
        ConnectionException ce("connection " + 
                               connectionId_ +
                               " has been closed");
        throw ce;
    }
    
    //sync the access to the internal queue
    //because it can be accessed in parallel
    //by other threads that execute the
    //onMessageSent callback
    {//enter critical section
        lock_guard<mutex> lock(mutex_);
        //save the message in the internal queue
        sentMessages_.push_back(message);
    }//exit critical section

    //create wrapper for callback member function
    std::string messageId = message.messageId;
    auto onMessageSentWrapper = [this, messageId]
                                (const error_code& ec,
                                 size_t nrBytes)
    {
        this->onMessageSent(ec, nrBytes, messageId);
    };
    
    boost::asio::async_write(socket_, 
                             buffer(sentMessages_.back().buffer),
                             onMessageSentWrapper);
}

void Connection::onMessageSent(const error_code& ec, 
                               size_t nrBytes,
                               string messageId)
{
    LOG <<INFO << Logger::trace
        << " Connection::onMessageSent "
        << messageId
        << " "
        << connectionId_
        << "\n";

    if (ec)
    {
        LOG << INFO << Logger::error
            << "Connection::onMessageSent "
            << ec.message()
            << "\n";

        //mark the connection as closed
        {//enter critical section
            lock_guard<mutex> lock(mutex_);
            closed_ = true;
        }//exit critical secation
    }

    //remove the message with the
    //specified id from the queue
    {//enter critical section
        lock_guard<mutex> lock(mutex_);
        
        deque<Message>::iterator messageIt;
        for (messageIt = sentMessages_.begin();
             messageIt != sentMessages_.end();
             ++messageIt)
        {
            if ((*messageIt).messageId == messageId)
            {
                sentMessages_.erase(messageIt);
                break;
            }
        }
    }//exit critical section
}

vector<Message> Connection::receiveMessages()
{
    vector<Message> output;
    
    //add completed messages
    {//enter critical section
        lock_guard<mutex> lock(mutex_);
        while (receivedMessages_.size() != 0)
        {
            if (receivedMessages_.front().complete == true)
            {
                output.push_back(receivedMessages_.front());
                receivedMessages_.pop_front();
            }
            else
            {
                LOG << INFO << Logger::trace
                    << receivedMessages_.front().messageId
                    << " the message has not been received\n";
                break;
            }
        }
    }//exit critical section
    
    return output;
}

string Connection::getMessageId()
{
   char tempBuffer[10];
   sprintf(tempBuffer, ":received:%d", messageCount_);
   ++messageCount_;
   string output(connectionId_ + string(tempBuffer));
   return output;
}

std::vector<Message> Connection::close()
{
    vector<Message> output;
    
    if (closed_) {
        return output;
    }

    {//enter critical section
        lock_guard<mutex> lock(mutex_);
        while (receivedMessages_.size() != 0) {
            if (receivedMessages_.front().complete) {
                output.push_back(receivedMessages_.front());
                receivedMessages_.pop_front();
            }
            else {
                break;
            }
        }

        closed_ = true;

    }//leave critical section

    //close the socket
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket_.close();
    
    return output;
}
