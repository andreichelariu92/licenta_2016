#ifndef FileEventServer_H_INCLUDE_GUARD
#define FileEventServer_H_INCLUDE_GUARD

//standard library headers
#include <array>
#include <string>
#include <iostream>
//boost library headers
#include <boost/asio.hpp>
//my headers
#include "DirectoryWatcher.h"
#include "../../util/src/Logger.h"
#include "Serializer.h"

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

//shorter version of boost
//nested namespaces
typedef boost::asio::local::stream_protocol stream_protocol;

///abstract class that represents a simple
///server using Unix Domain Sockets
///It accepts just a single connection.
class UnixSocketServer
{
protected:
    boost::asio::io_service& ioService_;
    stream_protocol::acceptor acceptor_;
    stream_protocol::socket socket_;
    std::array<char, 50> data_;

    void onAccept(const boost::system::error_code& ec);
    void onMessageReceived(const boost::system::error_code& ec,
                           size_t nrBytes);
    virtual void customOnAccept(const boost::system::error_code& ec)
        = 0;
    virtual void customOnMessageReceived(
            const boost::system::error_code& ec, size_t nrBytes) = 0;
public:
    UnixSocketServer(std::string filePath, 
                     boost::asio::io_service& ioService);
    //remove copy operations
    UnixSocketServer(const UnixSocketServer& source) = delete;
    UnixSocketServer& operator=(const UnixSocketServer& source)
        = delete;
    //move operations
    UnixSocketServer(UnixSocketServer&& source)=default;
    UnixSocketServer& operator=(UnixSocketServer&& source)=default;
    virtual ~UnixSocketServer()
    {}
};
class FileEventServer : public UnixSocketServer
{
private:
    int sendSeqNum_;
    int receiveSeqNum_;
    int timeout_;
    int windowSize_;
    DirectoryWatcher directoryWatcher_;
    Serializer serializer_;

    void customOnAccept(const boost::system::error_code& ec)override;
    void customOnMessageReceived(const boost::system::error_code& ec,
                                 size_t nrBytes)override;
    void sendFileEvents();
public:
    FileEventServer(std::string socketPath,
                    boost::asio::io_service& ioService,
                    std::string directoryPath,
                    int timeout = 60000,
                    int windowSize = 5);
    //remove copy operations
    FileEventServer(const FileEventServer& source) = delete;
    FileEventServer& operator=(const FileEventServer& source)
        = delete;
    //move operations
    FileEventServer(FileEventServer&& source)=default;
    FileEventServer& operator=(FileEventServer&& source)=default;

    virtual ~FileEventServer()
    {}
};
#else
#error NO UNIX DOMAIN SOCKETS
#endif

#endif
