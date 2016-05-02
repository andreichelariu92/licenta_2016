#ifndef Connection_H_INCLUDE_GUARD
#define Connection_H_INCLUDE_GUARD

//standard libraries
#include <deque>
#include <vector>
#include <string>
//boost libraries
#include <boost/asio.hpp>

//TODO: Andrei: create a message structure
//with messageId and completion flag

//namespace alias for long
//nested namespaces in boost
typedef boost::asio::ip::tcp tcp;
typedef boost::system::error_code error_code;
typedef std::vector<char> Message_t;

///Class that represent a connection
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
    std::deque<Message_t> receivedMessages_;
    std::deque<Message_t> sentMessages_;
    //async operations callbacks
    void onMessageReceived(const error_code& ec, size_t nrBytes);
    void onMessageSent(const error_code& ec, size_t nrBytes);
    //other private methods
    //prepare the buffer where the next
    //message received will be saved
    void prepareMessage();
public:
    Connection(boost::asio::io_service& ioService,
               std::string ip, 
               int port);
    //remove copy operations
    Connection(const Connection& source) = delete;
    Connection& operator=(const Connection& source) = delete;
    //default move operations
    Connection(Connection&& source) = default;
    Connection& operator=(Connection&& source) = default;
    //default destructor
    ~Connection() = default;

    void sendMessage(Message_t& message);
    std::vector<Message_t> receiveMessages();
};
#endif
