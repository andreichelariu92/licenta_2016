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
     nrThreads_(nrThreads)
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
    for (Connection& c : connections_)
    {
        c.sendMessage(m);
    }
}

vector<Message> BroadcastMechanism::readFromAll(int timeout)
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

    return output;
}