/**
 * @file Server.h
 * @author Andrei Chelariu
 */
#ifndef SERVER_H
#define SERVER_H
#include<boost/asio.hpp>
#include<boost/asio/buffer.hpp>
#include<boost/asio/read.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/asio/io_service.hpp>
#include"Session.h"
#include<list>
#include<AddressManager.h>
/**
  * @section DESCRIPTION
  * The ServerSettings structure
  * holds the settings to configure
  * the Server.
  * This structure is passed as a
  * parameter to the Server constructor.
  */
struct ServerSettings
{
    //members
    const unsigned int nrThreads;
    const unsigned int sessionSize;
    const unsigned int clientQueueSize;
    //functions
    ServerSettings(unsigned int argNrThreads=3, unsigned int argSessionSize=1024, unsigned int argClientQueueSize=3);
    static ServerSettings defaultSettings();
};

/**
 * @section DESCRIPTION
 *
 * The Server class listens for connections
 * on the specified port. When a new client connects,
 * it creates a Session object, which receives messages
 * from the client and does file operations according to
 * those messages. The server class does not have copy or
 * move operations.
 */
class Server
{
private:
    //private members
    boost::asio::io_service m_ioService;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::list<Session> m_sessions;
    int m_port;
    boost::asio::signal_set m_signalSet;
    AddrManager m_addrManager;
    ServerSettings m_serverSettings;
    //callback functions
    void on_clientConnect(Session& argSession, const boost::system::error_code& ec);
    void onExit(boost::system::error_code ec, int signal);
    void createSessionAndStartIt(unsigned int bufferSize);
public:
    ///Constructor
    Server(int argPort, AddrManager& argAddrMgr, ServerSettings argServerSettings={});

    //delete copy and move operations
    Server(const Server& source)=delete;
    Server& operator=(const Server& source)=delete;
    Server(Server&& source)=delete;
    Server& operator=(Server&& source)=delete;

    ///Start the server
    void start();

    ///Run the server on this thread
    /// BLOCKS untill all async operations
    /// are performed.
    void runSingleThread();

    ///Run the server on multiple threads
    /// this function creates the specified
    /// number of threads and calls io_service.run().
    ///This function does not call the run method
    /// on the calling thread
    void runMultiThread();
};

#endif // SERVER_H



