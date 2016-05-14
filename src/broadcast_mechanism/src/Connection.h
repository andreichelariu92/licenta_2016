#ifndef Connection_H_INCLUDE_GUARD
#define Connection_H_INCLUDE_GUARD

//standard libraries
#include <deque>
#include <vector>
#include <string>
#include <mutex>
//boost libraries
#include <boost/asio.hpp>

struct Message
{
    std::vector<char> buffer;
    bool complete;
    std::string messageId;
    
    Message(std::string argMessageId,
            unsigned int messageSize = 8*1024)
        :buffer(messageSize, ' '),
         complete(false),
         messageId(argMessageId)
    {}

    Message(std::vector<char> argBuffer,
            std::string argMessageId)
        :buffer(argBuffer),
         complete(false),
         messageId(argMessageId)
    {}

    Message(std::string stringBuffer,
            std::string argMessageId)
        :buffer(stringBuffer.begin(), stringBuffer.end()),
         complete(false),
         messageId(argMessageId)
    {}

};
//namespace alias for long
//nested namespaces in boost
typedef boost::asio::ip::tcp tcp;
typedef boost::system::error_code error_code;

///Class that represents a connection
///to a given pair of (ip, port).
///If the connection times out, an
///exception is being thrown in the
///constructor.
///The connection class can be used to
///send and to receive messages.
///The messages received are kept in an
///internal message queue and can be
///pooled by the client code.
class Connection
{
private:
    tcp::socket socket_;
    std::deque<Message> receivedMessages_;
    std::deque<Message> sentMessages_;
    std::deque<Message> errorMessages_;
    std::string connectionId_;
    unsigned int messageCount_;
    std::mutex mutex_;
    //async operations callbacks
    void onMessageReceived(const error_code& ec, size_t nrBytes);
    void onMessageSent(const error_code& ec, 
                       size_t nrBytes,
                       std::string messageId);
    //other private methods
    //prepare the buffer where the next
    //message received will be saved
    void prepareMessage();
    std::string getMessageId();
public:
    Connection(boost::asio::io_service& ioService,
               std::string ip, 
               int port,
               std::string connectionId);
    //remove copy operations
    Connection(const Connection& source) = delete;
    Connection& operator=(const Connection& source) = delete;
    //remove move operations
    Connection(Connection&& source) = delete;
    Connection& operator=(Connection&& source) = delete;
    //default destructor
    ~Connection() = default;

    void sendMessage(Message& message);
    std::vector<Message> receiveMessages();

    std::string connectionId()const
    {
        return connectionId_;
    }
};
#endif
