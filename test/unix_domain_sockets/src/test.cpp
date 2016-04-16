#include <boost/asio.hpp>
#include <iostream>
#include <array>
#include <string>
#include <cstdio>
#include <sstream>

using std::string;
using std::cout;
using std::stringstream;
//INSPIRATION
//http://www.boost.org/doc/libs/1_60_0/doc/html/boost_asio/example/cpp03/local/stream_client.cpp

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;
using boost::system::error_code;
class SimpleServer
{
private:
    boost::asio::io_service& ioService_;
    stream_protocol::acceptor acceptor_;
    stream_protocol::socket socket_;
    std::array<char, 256> data_;

    void onAccept(const boost::system::error_code& ec)
    {
        cout << "Client connected\n";
        stringstream ss;
        ss << "{\"path\" : \"/home/andrei\"}\n";
        /*
        auto onMessageReceivedWrapper = [this](const error_code& ec,
                                               size_t nrBytes)
        {
            this->onMessageReceived(ec, nrBytes);
        };
        socket_.async_read_some(boost::asio::buffer(data_),
                                onMessageReceivedWrapper);
        */

        boost::asio::write(socket_,
                           boost::asio::buffer(ss.str()));
    }

    void onMessageReceived(const error_code& ec, size_t nrBytes)
    {
        auto onMessageReceivedWrapper = [this](const error_code& ec,
                                               size_t nrBytes)
        {
            this->onMessageReceived(ec, nrBytes);
        };
        cout << "Received " << data_.data() << "\n";
        try
        {
        socket_.read_some(boost::asio::buffer(data_));
        socket_.async_read_some(boost::asio::buffer(data_),
                                onMessageReceivedWrapper);
        data_.fill(0);
        }
        catch(std::exception& e)
        {
            cout << e.what() << "\n";
        }
    }
public:
    SimpleServer(boost::asio::io_service& ioService,
                 const string& file)
        :ioService_(ioService),
         acceptor_(ioService_, stream_protocol::endpoint(file)),
         socket_(ioService_),
         data_()
    {
        auto onAcceptWrapper = [this](const error_code& ec)
        {
            this->onAccept(ec);
        };
        acceptor_.async_accept(socket_, onAcceptWrapper);
    }

};
int main()
{
    boost::asio::io_service ioService;
    std::remove("./file.txt");
    SimpleServer ss(ioService, "./file.txt");
    ioService.run();
    return 0;
}

#else
#error NO UNIX DOMAIN SOCKETS
#endif
