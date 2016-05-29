#ifndef BroadcastMechanism_H_INCLUDE_GUARD
#define BroadcastMechanism_H_INCLUDE_GUARD

//standard library headers
#include <list>
#include <thread>
#include <string>
#include <exception>
#include <vector>

//boost library headers
#include <boost/asio.hpp>

//my headers
#include "Connection.h"

///Class that manages a pool of
///threads and uses them to send
///and receive messages from multiple
///connections. The user needs to add
///at least one connection before calling
///the method start. All the threads are joined
///in the destructor of the class.
class BroadcastMechanism
{
private:
    std::list<Connection> connections_;
    std::vector<std::thread> threads_;
    boost::asio::io_service ioService_;
    int nrThreads_;
    void createAndStartThreads();
    void removeClosedConnections();
public:
    //constructor
    BroadcastMechanism(int nrThreads);
    //remove copy operations
    BroadcastMechanism(const BroadcastMechanism& source) = delete;
    BroadcastMechanism& operator=(const BroadcastMechanism& source)
        = delete;
    //remove move operations
    BroadcastMechanism(BroadcastMechanism&& source) = delete;
    BroadcastMechanism& operator=(BroadcastMechanism&& source)
        = delete;
    //destructor
    ~BroadcastMechanism();
    
    ///Method that adds a new connection
    ///to the broadcast mechanism. If it
    ///is the first connection added, it
    ///also starts the thread pool.
    void addConnection(std::string ip, 
                       int port,
                       std::string connectionId);
    ///Method that removes the connection
    ///with the specified id from the
    ///broadcast mechanism.
    void removeConnection(std::string connectionId);
    ///Method that sends a message to
    ///all the connections in the
    ///broadcast mechanism. The
    ///method returns immediately. If
    ///there are any errors, they will be
    ///reported when calling readFromAll
    void sendToAll(Message m);
    ///Method that waits the specified
    ///time in milliseconds and then
    ///reads the messages received by
    ///each connection. In the vector,
    ///there can also be error messages!
    std::vector<Message> readFromAll(int timeout);
};

#endif
