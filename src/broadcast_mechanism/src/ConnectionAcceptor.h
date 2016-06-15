#ifndef ConnectionAcceptor_INCLUDE_GUARD
#define ConnectionAcceptor_INCLUDE_GUARD

//standard libraries

//boost libraries
#include <boost/asio.hpp>

//my headers
#include "../../util/src/Logger.h"

//namespace alias for long
//nested namespaces in boost
typedef boost::asio::ip::tcp tcp;
typedef boost::system::error_code error_code;
namespace asio =  boost::asio;

template<typename ConnectionFunctor>
class ConnectionAcceptor
{
private:
    ///reference to the io_service used by boost asio
    ///to handle all the async operations
    asio::io_service& ioService_;
    ///acceptor used to receive tcp connections
    tcp::acceptor acceptor_;
    ///socket for the connection that is currently
    ///being waited to come
    tcp::socket pendingSocket_;
    ///function that has the signature void(tcp::socket)
    ///after a connection is being established successfully,
    ///this callback is called. The pendingSocket_ member will
    ///be emptied and will wait for a new connection
    ConnectionFunctor& connectionFunctor_;
    ///callback function 
    void onConnectionMade(const error_code& ec);
    ///function that waits for the next connection
    void waitConnection();
public:
    ConnectionAcceptor(asio::io_service& ioService,
                       ConnectionFunctor& cf);
    ///remove copy operations
    ConnectionAcceptor(const ConnectionAcceptor& soruce) = delete;
    ConnectionAcceptor& operator=(const ConnectionAcceptor& source)
      = delete;
    ///default move operations
    ConnectionAcceptor(ConnectionAcceptor&& source) = default;
    ConnectionAcceptor& operator=(ConnectionAcceptor&& source)
      = default;
    ///default destructor
    ~ConnectionAcceptor() = default;
    ///start the acceptor. When a new connection is being
    ///made on the given port, it calls the functor given
    ///as parameter to the contructor.
    void start(int port);
};

template<typename ConnectionFunctor>
ConnectionAcceptor<ConnectionFunctor>
    ::ConnectionAcceptor(asio::io_service& ioService,
                         ConnectionFunctor& cf)
    :ioService_(ioService),
     acceptor_(ioService),
     pendingSocket_(ioService_),
     connectionFunctor_(cf)
{
    acceptor_.open(tcp::v4());
}

template<typename ConnectionFunctor>
void ConnectionAcceptor<ConnectionFunctor>::waitConnection()
{
    //create a wrapper for the member function onConnectionMade
    auto onConnectionMadeWrapper = [this](const error_code& ec)
    {
        this->onConnectionMade(ec);
    };
    
    //start async operation
    acceptor_.async_accept(pendingSocket_, onConnectionMadeWrapper);
}

template<typename ConnectionFunctor>
void 
ConnectionAcceptor<ConnectionFunctor>::onConnectionMade(const error_code& ec)
{
    if (!ec) {
        //give the socket to the functor
        connectionFunctor_(std::move(pendingSocket_));
        //make a new socket
        pendingSocket_ = tcp::socket(ioService_);
        
        waitConnection();
    }
    else {
        LOG << INFO << Logger::error
            << "ConnectionAcceptor::onConnectionMade error"
            << "\n";
    }
}

template<typename ConnectionFunctor>
void 
ConnectionAcceptor<ConnectionFunctor>::start(int port)
{
    //configure acceptor
    asio::socket_base::reuse_address option(true);
    acceptor_.set_option(option);
    acceptor_.bind(tcp::endpoint(tcp::v4(), port));
    acceptor_.listen();
    //prepare socket for the first connection
    waitConnection();
}
#endif
