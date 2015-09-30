#include"Session.h"
#include<boost/asio.hpp>
#include<boost/asio/write.hpp>
#include<boost/asio/buffer.hpp>
//**************************************
//function definitions for Session class
//**************************************
Session::Session(boost::asio::io_service& argIoService, unsigned int bufferSize)
   :m_socket(argIoService), m_bufferSize(bufferSize), m_buffer(0), m_operationManager()
{
    m_buffer=new char[m_bufferSize];
}
Session::Session(Session&& source)
   :m_socket(std::move(source.m_socket)), m_bufferSize(source.m_bufferSize),
    m_buffer(source.m_buffer), m_operationManager(std::move(source.m_operationManager))
{
   source.m_buffer=0;
}
Session& Session::operator=(Session&& source)
{
    //delete the previous buffer
    if(m_buffer != 0)
        delete[] m_buffer;

    m_socket=std::move(source.m_socket);
    m_bufferSize=source.m_bufferSize;
    m_buffer=source.m_buffer;
    m_operationManager=std::move(source.m_operationManager);
    return *this;
}
Session::~Session()
{
    if(m_buffer != 0)
        delete[] m_buffer;
}
void Session::onMessageReceived(const boost::system::error_code& ec, std::size_t nrBytes)
{
    LOG<<Logger::Prio::trace<<INFO<<" onMessageReceived "<<ec.message()<<"\n";
    if(ec)
    {
        //exit the function
        LOG<<Logger::Prio::error<<INFO<<ec.message()<<"\n";
        return;
    }
    bool stopFlag=false;
    //create operation
    Operation* op=m_operationManager.createOperation(m_buffer,nrBytes);

    if(op != 0 )
    {
        //exec operation
        op->exec();

        //get the result
        std::string result=op->getResult();
        //send it  back synchronously
        try
        {
            boost::asio::write( m_socket, boost::asio::buffer(result.c_str(), result.size()) );

            //set the stopFlag
            stopFlag=op->requestStop();
        }
        catch(boost::system::system_error& e)
        {
            LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
            //stop listening for new messages
            stopFlag = true;
        }
       //delete the operation
       delete op;
    }

    //if there is no request to stop
    //continue to receive messages
    if(!stopFlag)
    {
        //when the next message is sent call onMessageReceived
        auto wrapper=[this](const boost::system::error_code& ec, std::size_t nrBytes)
        {
            this->onMessageReceived(ec,nrBytes);
        };
        //the function will exit imediately
        //it is not recursive!
        m_socket.async_read_some(boost::asio::buffer(m_buffer,m_bufferSize),wrapper);
    }
    else
    {
        //if there is a request to stop
        //close the Session
        try
        {
            this->close();
        }
        catch(boost::system::system_error& e)
        {
            LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
        }
    }
}
void Session::close()
{
    if(m_buffer)
    {
        delete m_buffer;
        m_buffer=0;
    }
    if(m_socket.is_open())
    {
        LOG<<Logger::Prio::trace<<INFO<<" close session\n";
        //close the read
        //and write buffers
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        //close the socket
        m_socket.close();
    }
}

bool Session::processMessage(std::size_t nrBytes)
{

   //send the message back to the client
   //this function will block the thread
   //untill is executed!
   boost::asio::write(m_socket, boost::asio::buffer(m_buffer,nrBytes));
   char message[nrBytes];
   strncpy(message,m_buffer,nrBytes);

   if(strstr(message, "quit") != 0)
       return false;
   else
       return true;
}
