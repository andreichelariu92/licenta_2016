#include"Server.h"
#include<iostream>
#include<csignal>
#include<thread>

using namespace boost::asio;
using namespace boost::asio::ip;
ServerSettings::ServerSettings(unsigned int argNrThreads, unsigned int argSessionSize, unsigned int argClientQueueSize)
   :nrThreads(argNrThreads),
    sessionSize(argSessionSize),
    clientQueueSize(argClientQueueSize)
{}
ServerSettings ServerSettings::defaultSettings()
{
    return ServerSettings{};
}

//**************************************
//function definitions for EchoServer class
//**************************************
Server::Server(int argPort, AddrManager& addrMgr, ServerSettings argServerSettings)
  :m_ioService(), m_acceptor(m_ioService),
   m_port(argPort),
   m_signalSet(m_ioService),
   m_addrManager(std::move(addrMgr)),
   m_serverSettings(argServerSettings),
   m_mutex()
{
    //register the exit signals
    //to the signal set
    m_signalSet.add(SIGINT);
    m_signalSet.add(SIGTERM);
#if defined(SIGQUIT)
    m_signalSet.add(SIGQUIT);
#endif

    //add wrapper to call the
    //member function
    //onExit when one of the
    //above signals is received
    auto exitWrapper=[this](boost::system::error_code ec, int signal)
    {
        this->onExit(ec,signal);
    };
    m_signalSet.async_wait(exitWrapper);

}
void Server::on_clientConnect(Session& argSession, const boost::system::error_code& ec)
{
   if(ec)
   {
        //exit the function if there is an error
    LOG<<Logger::Prio::error<<INFO<<ec.message()<<"\n";
    return;
   }
   //show message at log
   LOG<<Logger::Prio::trace<<INFO<<"Client connected \n";
   try
   {
    //check if the client has its ip
        //in the AddrManager
        //if it is receive messages from it
        std::string clientIp = AddrManager::getIp(argSession.getSocket());
        if( m_addrManager.findIp(clientIp) )
        {
            //create wrapper for the member session function
            auto wrapper=[&argSession](const boost::system::error_code& ec, std::size_t nrBytes)
            {
                argSession.onMessageReceived(ec,nrBytes);
            };

            //when the client sends a message
            //call argSession.onMessageReceived
            argSession.getSocket().async_read_some( boost::asio::buffer( argSession.getBuffer(), argSession.getBufferSize() ),wrapper);
            LOG<<Logger::Prio::trace<<INFO<<"End of on_clientConnect\n";
        }
        else
        {
            std::cout<<clientIp<<" is not in the trusted addresses\n";
            argSession.close();
        }
   }
   catch(boost::system::system_error& e)
   {
        LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
        try
        {
            argSession.close();
        }
        catch(boost::system::system_error& e)
        {
           LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
        }
   }

        //create session for the next client
        //and start it
        createSessionAndStartIt(m_serverSettings.sessionSize);
}
void Server::onExit(boost::system::error_code ec, int signal)
{
    if(!ec)
    {
        std::cout<<"\nthe server will exit\n";
        //close the acceptor
        try
        {
            m_acceptor.cancel();
            m_acceptor.close();
        }
        catch(boost::system::system_error& e)
        {
            LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
        }
    }
}
void Server::createSessionAndStartIt(unsigned int bufferSize)
{
   //get the mutex (serialize access)
   std::lock_guard<std::mutex> lock(m_mutex);

   //create a session for the next client
   Session leSession(m_ioService, bufferSize);

   //move the session in the list
   m_sessions.push_back( std::move(leSession) );

   //create wrapper for the member function
   auto wrapper=[this](const boost::system::error_code& ec)
   {
       this->on_clientConnect(m_sessions.back(), ec);
   };

   //when the next client connects
   //on_clientConnect will be called
   m_acceptor.async_accept(m_sessions.back().getSocket(),wrapper);
}
void Server::start()
{
    //create endpoint
   tcp::endpoint ep(tcp::v4(),m_port);

   try
   {
       //open the acceptor for the endpoint protocol
       m_acceptor.open(ep.protocol());

       //set the acceptor
        //to reuse the address
        boost::asio::socket_base::reuse_address reuseAddr(true);
        m_acceptor.set_option(reuseAddr);

       //bind acceptor with endpoint
       m_acceptor.bind(ep);

       //set the max client queue
       m_acceptor.listen(m_serverSettings.clientQueueSize);

       //create a session
       //for the first client and start it
       createSessionAndStartIt(m_serverSettings.sessionSize);
   }
   catch(boost::system::system_error& e)
   {
       LOG<<Logger::Prio::exception<<INFO<<std::string( e.what() );
   }
}
void Server::runSingleThread()
{
   m_ioService.run();
}
void Server::runMultiThread()
{
    using std::thread;

   //create a wrapper to
   //call io_service.run()
   auto wrapper=[this]()
   {
       this->m_ioService.run();
   };

   //call the run() method
   //on the remaining threads
   std::vector<thread> threads;
   for(unsigned int i=0; i<m_serverSettings.nrThreads; ++i)
   {
      std::thread t(wrapper);
      //move the thread in the vector
      threads.push_back(std::move(t));
   }

   m_ioService.run();

   //wait for all the threads to finish
   for(auto& t: threads)
       t.join();

}
