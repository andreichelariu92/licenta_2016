#include <iostream>
#include <vector>

#include "FileEventServer.h"
#include "../../util/src/Logger.h"

using std::string;
using std::cout;
using std::vector;

using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::buffer;
using boost::asio::write;

UnixSocketServer::UnixSocketServer(string filePath,
                                   io_service& ioService)
    :ioService_(ioService),
     acceptor_(ioService, stream_protocol::endpoint(filePath)),
     socket_(ioService),
     data_()
{
    //create wrapper for the member class onAccept
    auto onAcceptWrapper = [this](const error_code& ec)
    {
        this->onAccept(ec);
    };

    //start an asynchronous accept operation
    acceptor_.async_accept(socket_, onAcceptWrapper);
}

void UnixSocketServer::onAccept(const error_code& ec)
{
    LOG << INFO << Logger::trace
        << "client connected\n";
    
    //create wrapper for onMessageReceived method
    auto onMessageReceivedWrapper = [this](const error_code& ec,
                                           size_t nrBytes)
    {
        this->onMessageReceived(ec, nrBytes);
    };
    //start an asynchronous read operation
    socket_.async_read_some(buffer(data_),
                            onMessageReceivedWrapper);

    //call the virtual method customOnAccept,
    //which will be implemented by the child classes
    customOnAccept(ec);
}

void UnixSocketServer::onMessageReceived(const error_code& ec,
                                         size_t nrBytes)
{
    LOG << INFO << Logger::trace
        << "UnixSocketServer::onMessageReceived\n";
    //create wrapper for member callback
    auto onMessageReceivedWrapper = [this](const error_code& ec,
                                           size_t nrBytes)
    {
        this->onMessageReceived(ec, nrBytes);
    };
    
    //cal the virtual method customOnMessageReceived
    customOnMessageReceived(ec, nrBytes);

    //prepare the reading of the next message
    try
    {
        //try to read from the unix socket
        //if there is no exception, start
        //an asynchronous read
        socket_.read_some(buffer(data_));
        customOnMessageReceived(ec, data_.size());
        //clear all the current data, so
        //that the callback can work only
        //on the data from the next read
        data_.fill(0);
        socket_.async_read_some(buffer(data_),
                                onMessageReceivedWrapper);
    }
    catch(...)
    {
        LOG << INFO << Logger::error
            << "File closed\n";
    }
}

FileEventServer::FileEventServer(string socketPath,
                                 io_service& ioService,
                                 string directoryPath,
                                 int timeout,
                                 int windowSize)
    :UnixSocketServer(socketPath, ioService),
     sendSeqNum_(0),
     receiveSeqNum_(0),
     timeout_(timeout),
     windowSize_(windowSize),
     directoryWatcher_(directoryPath),
     serializer_()
{
}

void FileEventServer::customOnAccept(const error_code& ec)
{
    sendFileEvents();
}

void FileEventServer::customOnMessageReceived(const error_code& ec,
                                              size_t nrBytes)
{
    LOG << INFO << Logger::trace
        << "receiveSeqNum " << receiveSeqNum_ 
        << "\n";
    ++receiveSeqNum_;
    sendFileEvents();
}

void FileEventServer::sendFileEvents()
{
    //compute the number of reads the
    //directory watcher has to do
    //example 1:
    //windowSize_ = 5 sendSeqNum_ = 0 receiveSeqNum_ = 0
    //perform 5 reads
    //example 2:
    //windowSize_ = 5 sendSeqNum_ = 10 receiveSeqNum_ = 5
    //perform 0 reads
    const int numReads = windowSize_ - 
                         (sendSeqNum_ - receiveSeqNum_);

    int readIdx;
    for (readIdx = 0; readIdx < numReads; ++readIdx)
    {
        vector<FileEvent> fEvents = 
            directoryWatcher_.readEvents(timeout_);
        
        if (fEvents.size() != 0)
        {
            LOG << INFO << Logger::debug
                << "file events send "
                << fEvents.size()
                << "\n";

            string message = serializer_.serialize(fEvents);
            try
            {
                write(socket_, buffer(message));
                ++sendSeqNum_;
            }
            catch(std::exception& e)
            {
                LOG << INFO << Logger::error
                    << e.what();
            }
        }
    }
}
