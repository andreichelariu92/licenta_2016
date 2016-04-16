#include <iostream>

#include "FileEventServer.h"

using std::string;
using std::cout;

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
    //TODO: Andrei: log instead of STDOUT
    std::cout << "Client connected\n";
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
        //clear all the current data, so
        //that the callback can work only
        //on the data from the next read
        data_.fill(0);
        socket_.async_read_some(buffer(data_),
                                onMessageReceivedWrapper);
    }
    catch(...)
    {
    }
}

FileEventServer::FileEventServer(string socketPath,
                                 io_service& ioService,
                                 string directoryPath)
    :UnixSocketServer(socketPath, ioService),
     directoryWatcher_(directoryPath)
{
}

void FileEventServer::customOnAccept(const error_code& ec)
{
    string welcome("welcome\n");
    write(socket_, buffer(welcome));
}

void FileEventServer::customOnMessageReceived(const error_code& ec,
                                              size_t nrBytes)
{
    //TODO: Andrei: log instead of STDOUT
    cout << data_.data();
}
