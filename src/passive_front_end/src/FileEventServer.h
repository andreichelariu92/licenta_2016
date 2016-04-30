#ifndef FileEventServer_H_INCLUDE_GUARD
#define FileEventServer_H_INCLUDE_GUARD

//standard library headers
#include <string>
#include <vector>
//boost library headers
#include <boost/asio.hpp>
//my headers
#include "DirectoryWatcher.h"
#include "../../util/src/Logger.h"
#include "Serializer.h"
#include "UnixSocketServer.h"

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

class FileEventSession
{
private:
    tcp::socket socket_;
    std::vector<char> data_;
    DirectoryWatcher directoryWatcher_;
    int windowSize_;
    int sendSeqNum_;
    int receiveSeqNum_;
    int timeout_;
    Serializer serializer_;
    unsigned int sessionId_;

    void sendFileEvents();
    void processMessage(const boost::system::error_code& ec,
                        size_t nrBytes);
public:
    FileEventSession(boost::asio::io_service& ioService,
                     std::string directoryPath,
                     int timeout,
                     int windowSize,
                     unsigned int sessionId);
    //remove copy operations
    FileEventSession(const FileEventSession& source) = delete;
    FileEventSession& operator=(const FileEventSession& source)=delete;
    //keep the default move operations
    FileEventSession(FileEventSession&& source) = default;
    FileEventSession& operator=(FileEventSession&& source) = default;
    //custom destructor
    ~FileEventSession() = default;
    
    void onMessageReceived(const boost::system::error_code& ec,
                           size_t nrBytes);

    tcp::socket& socket()
    {
        return socket_;
    }
    std::vector<char>& data()
    {
        return data_;
    }

};

class FileEventSessionFactory
{
private:
    std::string directoryPath_;
    int timeout_;
    int windowSize_;
public:
    FileEventSessionFactory(std::string directoryPath,
                            int timeout = 1000,
                            int windowSize = 3);
    FileEventSession operator()(boost::asio::io_service& ioService,
                                unsigned int sessionId);
};

typedef UnixSocketServer<FileEventSession, FileEventSessionFactory> FileEventServer;
#else
#error NO UNIX DOMAIN SOCKETS
#endif

#endif
