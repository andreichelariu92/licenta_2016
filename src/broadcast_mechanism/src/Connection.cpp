#include "Connection.h"
#include "../../util/src/Logger.h"

using std::string;
using std::vector;
using std::deque;
using std::lock_guard;
using std::mutex;

namespace asio = boost::asio;
using boost::asio::buffer;

Connection::Connection(asio::io_service& ioService,
                       string ip,
                       int port,
                       string connectionId)
    :socket_(ioService),
     receivedMessages_(),
     sentMessages_(),
     errorMessages_(),
     connectionId_(connectionId),
     messageCount_(0),
     mutex_()
{
    using boost::asio::ip::address;
    tcp::endpoint endPoint(address::from_string(ip), port);
    socket_.connect(endPoint);
    
    prepareMessage();
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
        << "Connection::onMessageReceived "
        << "\n";
    
    if (!ec)
    {
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
        //remove the message because
        //an error occured
        receivedMessages_.pop_back();
       
        LOG << INFO << Logger::error
            << "Connection::onMessageReceived "
            << ec.message()
            << "\n";
    }
}

void Connection::sendMessage(Message& message)
{
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
    if (ec)
    {
        LOG << INFO << Logger::error
            << "Connection::onMessageSent "
            << ec.message()
            << "\n";

        //put an error message with the same
        //id in the error queue
        Message m("ERROR", messageId);
        {//enter critical section
            lock_guard<mutex> lock(mutex_);
            errorMessages_.push_back(m);
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
    
    //add error messages
    {//enter critical section
        lock_guard<mutex> lock(mutex_);

        while (errorMessages_.size() != 0)
        {
            output.push_back(errorMessages_.front());
            errorMessages_.pop_front();
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
