#ifndef SESSION_H
#define SESSION_H

#include<boost/asio/ip/tcp.hpp>
#include<boost/asio/buffer.hpp>
#include<boost/asio/io_service.hpp>


#include"OperationManager.h"
#include"Operation.h"
#include"../util/Logger.h"

class Session
{
private:
    boost::asio::ip::tcp::socket m_socket;
    unsigned int m_bufferSize;
    char* m_buffer;
    bool processMessage(std::size_t nrBytes);
    OperationManager m_operationManager;
public:
    //constructor, destructor and move operations
    Session(boost::asio::io_service& argIoService, unsigned int bufferSize);
    Session(Session&& source);
    Session& operator=(Session&& source);
    ~Session();
    //delete the copy operations because boost sockets
    //don't suppport them
    Session(const Session& source)=delete;
    Session& operator=(const Session& source)=delete;
    //this function will be called when a message
    //is received
    void onMessageReceived(const boost::system::error_code& ec, std::size_t nrBytes);
    //close the session
    void close();
    //get methods
    boost::asio::ip::tcp::socket& getSocket(){return m_socket;}
    char* getBuffer(){return m_buffer;}
    unsigned int getBufferSize(){return m_bufferSize;}

};

#endif // SESSION_H
