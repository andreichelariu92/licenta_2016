#include "BroadcastMechanism.h"

#include <iostream>
#include <thread>
#include <chrono>

using boost::asio::io_service;
using std::string;
using std::vector;
using namespace std::this_thread;

int main()
{
    //create broadcast mechanism
    //and add a connection to it
    BroadcastMechanism bm(2);
    /*
    //ip of the man7.org website
    bm.addConnection("213.131.240.174", 80, "connection1");
    //ip of the catb.org website
    bm.addConnection("152.19.134.41", 80, "connection2");

    //send a message containing a HTTP
    //request to all the connections
    string messageString("GET /\r\n");
    Message message(messageString, "connection1:send:1");
    bm.sendToAll(message);
    
    //try multiple reads, untill
    //the response to the request
    //is received
    unsigned int messageTry = 0;
    while (messageTry < 12)
    {
        vector<Message> messages = bm.readFromAll(5000);
        std::cout << "Message try " << messageTry << "\n";
        for (Message& m : messages)
        {
            std::cout << "MessageId = " << m.messageId << "\n";
            for (char c : m.buffer)
            {
                std::cout << c;
            }
            std::cout << "\n";
        }
        ++messageTry;
    }
    */

    bm.addConnection("127.0.0.1", 12345, "connection1");
    bm.addConnection("127.0.0.1", 54321, "connection2");
    
    int messageTry = 0;
    while (messageTry < 4)
    {
        Message m("message\n", "id");
        bm.sendToAll(m);

        vector<Message> messages = bm.readFromAll(5000);
        std::cout << "Message try " << messageTry << "\n";
        for (Message& m : messages)
        {
            std::cout << "MessageId = " << m.messageId << "\n";
            for (char c : m.buffer)
            {
                std::cout << c;
            }
            std::cout << "\n";
        }
        ++messageTry;

    }
    return 0;
}
