#include "Connection.h"
#include "../../util/src/Logger.h"

using std::string;
using std::vector;
using std::deque;

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
     messageCount_(0)
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
    
    //TODO: Andrei: sync access on
    //receivedMessages_
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
    //TODO: Andrei: sync access on sentMessages_
    
    //save the message in the internal queue
    sentMessages_.push_back(message);
    //create wrapper for callback member function
    auto onMessageSentWrapper = [this](const error_code& ec,
                                       size_t nrBytes)
    {
        this->onMessageSent(ec, nrBytes);
    };
    
    boost::asio::async_write(socket_, 
                             buffer(sentMessages_.back().buffer),
                             onMessageSentWrapper);
}

void Connection::onMessageSent(const error_code& ec, size_t nrBytes)
{
    //check messageId
    if (ec)
    {
        LOG << INFO << Logger::error
            << "Connection::onMessageSent "
            << ec.message()
            << "\n";

        //TODO: Andrei
        //add error message in receivedMessages_
        //sync access on receivedMessages_
    }
    
    //TODO: Andrei: sync access on sentMessages_
    sentMessages_.pop_back();
}

vector<Message> Connection::receiveMessages()
{
    //TODO: Andrei: sync access on receivedMessages_
    vector<Message> output;

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
