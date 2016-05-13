#include "Connection.h"

#include <iostream>
#include <thread>
#include <chrono>

using boost::asio::io_service;
using std::string;
using std::vector;
using namespace std::this_thread;

int main()
{
    //create an io_service
    //and start it
    io_service ioService;
    auto work = [&ioService]()
    {
        ioService.run();
    };
    
    //ip of the man7.org website
    Connection c(ioService, "213.131.240.174", 80, "connection1");
    std::thread t(work);
    string messageString("GET /\r\n");
    Message message(messageString, "connection1:send:1");
    c.sendMessage(message);
    
    unsigned int messageTry = 0;
    while (messageTry < 12)
    {
        vector<Message> messages = c.receiveMessages();
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

        sleep_for(std::chrono::seconds(5));
        ++messageTry;
    }

    t.join();
    return 0;
}
