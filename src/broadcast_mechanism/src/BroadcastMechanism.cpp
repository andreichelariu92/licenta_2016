#include <chrono>

#include "BroadcastMechanism.h"
#include "../../util/src/Logger.h"

using std::thread;
using std::list;
using std::string;
using std::vector;

BroadcastMechanism::BroadcastMechanism(int nrThreads)
    :connections_(),
     threads_(),
     ioService_(),
     nrThreads_(nrThreads),
     connectionAcceptor_(ioService_, *this)
{}

void BroadcastMechanism::createAndStartThreads()
{
    auto task = [this]()
    {
        this->ioService_.run();
    };

    int threadIdx;
    for (threadIdx = 0;
         threadIdx < nrThreads_;
         ++threadIdx)
    {
        thread t(task);
        threads_.push_back(std::move(t));
    }
}

BroadcastMechanism::~BroadcastMechanism()
{
    //TODO: Andrei: Think of a solution
    //to force the exit of threads
    for (thread& t : threads_)
    {
        t.join();
    }
}

void BroadcastMechanism::addConnection(string ip,
                                       int port,
                                       string connectionId)
{
    connections_.emplace_back(ioService_,
                              ip,
                              port,
                              connectionId);

    if (connections_.size() == 1)
    {
        createAndStartThreads();
    }
}

void BroadcastMechanism::removeConnection(string connectionId)
{
    //TODO: Andrei: log instead of cout
    /*
    std::cout << "removeConnection called " << connectionId << "\n";
    */
    list<Connection>::iterator connectionIt;
    for (connectionIt = connections_.begin();
         connectionIt != connections_.end();
         ++connectionIt)
    {
        if ((*connectionIt).connectionId() == connectionId)
        {
            connections_.erase(connectionIt);
            break;
        }
    }
}

void BroadcastMechanism::sendToAll(Message m)
{
    removeClosedConnections();

    for (Connection& c : connections_)
    {
        //in case any connection has been closed
        //after calling the removeClosedConnections
        //function
        try
        {
            c.sendMessage(m);
        }
        catch(std::exception& e)
        {
            LOG << INFO << Logger::error
                << "BroadcastMechanism::sendToAll "
                << e.what()
                << "\n";
        }
    }
}

vector<Message> BroadcastMechanism::receiveFromAll(int timeout)
{
    //block the current thread for
    //the specified ammount of time
    using std::this_thread::sleep_for;
    sleep_for(std::chrono::milliseconds(timeout));
    
    //get all the messages received
    //by each connection and put it
    //in the output vector
    vector<Message> output;
    for (Connection& c : connections_)
    {
        vector<Message> receivedMessages = c.receiveMessages();
        output.insert(output.end(),
                      receivedMessages.begin(),
                      receivedMessages.end());
    }
    
    //remove the closed connections after
    //reading from them because they may
    //have messages
    removeClosedConnections();

    return output;
}

void BroadcastMechanism::removeClosedConnections()
{
    auto closedPredicate = [](const Connection& c)
    {
        return c.closed();
    };

    connections_.remove_if(closedPredicate);
}

void BroadcastMechanism::operator()(tcp::socket s)
{
    const string connectionId = "connection" + 
                                std::to_string(connections_.size());
    connections_.emplace_back(ioService_,
                              std::move(s),
                              connectionId);
}

void BroadcastMechanism::startAccept(int port)
{
    connectionAcceptor_.start(port);
}