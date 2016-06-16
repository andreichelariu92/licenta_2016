#include <iostream>
#include <vector>
#include <algorithm>

#include "FileEventServer.h"
#include "../../util/src/Logger.h"

using std::string;
using std::cout;
using std::vector;

using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::buffer;
using boost::asio::write;


void FileEventSession::onMessageReceived(const error_code& ec,
                                         size_t nrBytes)
{
    LOG << INFO << Logger::trace
        << "FileEventSession::onMessageReceived"
        << " sessionId = " << sessionId_
        << "\n";
    //create wrapper for member callback
    auto onMessageReceivedWrapper = [this](const error_code& ec,
                                           size_t nrBytes)
    {
        this->onMessageReceived(ec, nrBytes);
    };
    processMessage(ec, nrBytes);

    //prepare the reading of the next message
    try
    {
        //clear all the current data, so
        //that the callback can work only
        //on the data from the next read
        std::fill(data_.begin(), data_.end(), 0);
        socket_.async_read_some(buffer(data_),
                                onMessageReceivedWrapper);
    }
    catch(...)
    {
        LOG << INFO << Logger::error
            << "File closed\n";
    }
}

FileEventSession::FileEventSession(io_service& ioService,
                                   string directoryPath,
                                   int timeout,
                                   int windowSize,
                                   unsigned int sessionId)
    :socket_(ioService),
     data_(100, 0),
     directoryWatcher_(directoryPath),
     windowSize_(windowSize),
     sendSeqNum_(0),
     receiveSeqNum_(0),
     timeout_(timeout),
     serializer_(),
     sessionId_(sessionId)
{
}

void FileEventSession::processMessage(const error_code& ec,
                                     size_t nrBytes)
{
    if (!ec)
    {
        LOG << INFO << Logger::trace
            << "receiveSeqNum " << receiveSeqNum_ 
            << "\n";
        ++receiveSeqNum_;
        sendFileEvents();
    }
    else
    {
        LOG << INFO << Logger::error
            << "FileEventServer::processMessage "
            << ec.message()
            << "\n";
    }
}

void FileEventSession::sendFileEvents()
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

FileEventSessionFactory::FileEventSessionFactory(string dirPath,
                                                 int timeout,
                                                 int windowSize)
    :directoryPath_(dirPath),
     timeout_(timeout),
     windowSize_(windowSize)
{}

FileEventSession 
FileEventSessionFactory::operator()(io_service& ioService,
                                    unsigned int sessionId)
{
    return FileEventSession(ioService,
                            directoryPath_,
                            timeout_,
                            windowSize_,
                            sessionId);
}
