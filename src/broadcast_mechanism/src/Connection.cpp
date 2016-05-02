#include "Connection.h"
#include "../../util/src/Logger.h"

using std::string;
using std::vector;
using std::deque;

namespace asio = boost::asio;
using boost::asio::buffer;

Connection::Connection(asio::io_service& ioService,
                       string ip,
                       int port)
    :socket_(ioService),
     receivedMessages_(),
     sentMessages_()
{
    using boost::asio::ip::address;
    tcp::endpoint endPoint(address::from_string(ip), port);
    socket_.connect(endPoint);
    
    prepareMessage();
}

void Connection::prepareMessage()
{
    //create buffer to hold the message
    //for the first client
    Message_t message(8*1024, '#');
    receivedMessages_.push_back(std::move(message));
    //create wrapper for member function
    auto onMessageReceivedWrapper = [this](const error_code& ec,
                                           size_t nrBytes)
    {
        this->onMessageReceived(ec, nrBytes);
    };
    //start async read operation
    socket_.async_read_some(buffer(receivedMessages_.back()),
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
        Message_t& message = receivedMessages_.back();
        //resize the vetor to keep only
        //the date received from the network
        message.resize(nrBytes);

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

void Connection::sendMessage(Message_t& message)
{
    //save the message in the internal queue
    sentMessages_.push_back(message);
    //create wrapper for callback member function
    auto onMessageSentWrapper = [this](const error_code& ec,
                                       size_t nrBytes)
    {
        this->onMessageSent(ec, nrBytes);
    };
    
    boost::asio::async_write(socket_, 
                             buffer(sentMessages_.back()),
                             onMessageSentWrapper);
}

void Connection::onMessageSent(const error_code& ec, size_t nrBytes)
{
    if (ec)
    {
        LOG << INFO << Logger::error
            << "Connection::onMessageSent "
            << ec.message()
            << "\n";

        //TODO: Andrei
        //add error message in receivedMessages_
    }
    
    sentMessages_.pop_back();
}

vector<Message_t> Connection::receiveMessages()
{
    vector<Message_t> output;

    while (receivedMessages_.size() != 0)
    {
        //check if the message has been
        //read completly(i.e. size != 8KO
        //and first character != '#')
        const int messageSize = receivedMessages_.front().size();
        const char firstChar = receivedMessages_.front()[0];
        
        if (messageSize != 8*1024
            &&
            firstChar != '#')
        {
            output.push_back(receivedMessages_.front());
            receivedMessages_.pop_front();
        }
        else
        {
            //TODO: Andrei: add message id
            LOG << INFO << Logger::trace
                << " the message has not been received\n";
            break;
        }
    }

    return output;
}
