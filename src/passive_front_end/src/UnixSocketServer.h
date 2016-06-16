#ifndef UnixSocketServer_H_INCLUDE_GUARD
#define UnixSocketServer_H_INCLUDE_GUARD

//standard library headers
#include <list>
#include <string>
#include <iostream>
//boost library headers
#include <boost/asio.hpp>
//my headers
#include "../../util/src/Logger.h"


#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

//shorter version of boost
//nested namespaces
typedef boost::asio::ip::tcp tcp;
namespace asio =  boost::asio;

template<typename Session, typename SessionFactory>
class UnixSocketServer
{
private:
    std::list<Session> sessions_;
    asio::io_service& ioService_;
    tcp::acceptor acceptor_;
    SessionFactory factory_;
    unsigned int sessionId_;

    void prepareSession();
    void onAccept(Session& currentSession,
                  const boost::system::error_code& ec);
public:
    UnixSocketServer(asio::io_service& ioService,
                     int port,
                     SessionFactory factory);
    //remove copy operations
    UnixSocketServer(const UnixSocketServer& source) = delete;
    UnixSocketServer& operator=(const UnixSocketServer& source)
        = delete;
    //default move operations
    UnixSocketServer(UnixSocketServer&& source) = default;
    UnixSocketServer& operator=(UnixSocketServer&& source) = default;
    //default destructor
    ~UnixSocketServer() = default;
};

template<typename Session, typename SessionFactory>
void UnixSocketServer<Session, SessionFactory>::prepareSession()
{
    //Create a session for the next client
    Session s = factory_(ioService_, sessionId_);
    ++sessionId_;
    //move the session to the list
    sessions_.push_back(std::move(s));
    //create wrapper for member method onAccept
    auto onAcceptWrapper = [this](const boost::system::error_code& ec)
    {
        this->onAccept(this->sessions_.back(), ec);
    };

    //start an asynchronous accept operation
    acceptor_.async_accept(sessions_.back().socket(),
                           onAcceptWrapper);
}

template<typename Session, typename SessionFactory>
UnixSocketServer<Session, SessionFactory>
    ::UnixSocketServer(asio::io_service& ioService,
                       int port,
                       SessionFactory factory)
    :sessions_(),
     ioService_(ioService),
     acceptor_(ioService_, tcp::endpoint(tcp::v4(), port)),
     factory_(factory),
     sessionId_(0)
{
    asio::socket_base::reuse_address option(true);
    acceptor_.set_option(option);
    //prepare session for the first client
    prepareSession();
}

template<typename Session, typename SessionFactory>
void UnixSocketServer<Session, SessionFactory>
    ::onAccept(Session& currentSession,
               const boost::system::error_code& ec)
{
    using asio::buffer;
    if (!ec)
    {
        LOG << INFO << Logger::debug
            << " UnixSocketServer::onAccept "
            << "client connected\n";

        //start an asynchronous read operation
        auto onMessageReceivedWrapper = 
            [&currentSession](const boost::system::error_code& ec, 
                              size_t nrBytes)
        {
            currentSession.onMessageReceived(ec, nrBytes);
        };
        currentSession.socket().async_read_some(
            buffer(currentSession.data()), onMessageReceivedWrapper);
    }
    else
    {
        LOG << INFO << Logger::error
            << " UnixSocketServer::onAccept "
            << ec.message()
            << "\n";
    }
    //prepare session for the next client
    prepareSession();
}
#else
#error NO UNIX DOMAIN SOCKETS
#endif

#endif
