#include "BroadcastMechanism.h"
#include "LuaWrapper.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>

extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
using boost::asio::io_service;
using std::string;
using std::vector;
using namespace std::this_thread;

static BroadcastMechanism g_bcast(2);

int testFunc(lua_State* L)
{
    LuaInterpreter lua(L);
    try
    {
        string message = lua.getString(1);
        std::cout << "From native side " << message << "\n";
        return 0;
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
        return 0;
    }

}

int bcastInit(lua_State* L)
{
    LuaInterpreter lua(L);
    
    try
    {
        int port = lua.getNumber(1);
        g_bcast.startAccept(port);
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
    }
    
    return 0;
}
int addConnection(lua_State* L)
{
    LuaInterpreter lua(L);

    try
    {
        string ip = lua.getString(1);
        int port = (int)lua.getNumber(2);
        string connectionId = lua.getString(3);

        g_bcast.addConnection(ip, port, connectionId);
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
    }

    return 0;
}

int removeConnection(lua_State* L)
{
    LuaInterpreter lua(L);

    try
    {
        string connectionId = lua.getString(1);
        g_bcast.removeConnection(connectionId);
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
    }

    return 0;
}

int sendToAll(lua_State* L)
{
    LuaInterpreter lua(L);
    
    try
    {
        string messageBuffer = lua.getString(1);
        string messageId = lua.getString(2);
        
        Message m(messageBuffer, messageId);
        g_bcast.sendToAll(m);
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
    }
    
    return 0;
}

int receiveFromAll(lua_State* L)
{
    LuaInterpreter lua(L);
    
    try
    {
      
      //get timeout
      int timeout = lua.getNumber(1);
      
      std::vector<Message> messages = g_bcast.receiveFromAll(timeout);
      lua.pushMessages(messages);
      
      return 1;
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
        return 0;
    }
}

int bcastDeinit(lua_State* L)
{
    LuaInterpreter lua(L);

    try
    {
        vector<Message> finishedMsg = g_bcast.closeConnections();
        lua.pushMessages(finishedMsg);

        return 1;
    }
    catch(std::exception& e)
    {
        luaL_error(L, "%s", e.what());
        return 0;
    }
}

extern "C" {
    int luaopen_broadcast_mechanism(lua_State* L)
    {
        LuaInterpreter lua(L);
        vector<LuaCFunction> functions;
        functions.emplace_back(testFunc, "show");
        functions.emplace_back(addConnection, "addConnection");
        functions.emplace_back(removeConnection, "removeConnection");
        functions.emplace_back(sendToAll, "sendToAll");
        functions.emplace_back(receiveFromAll, "receiveFromAll");
        functions.emplace_back(bcastInit, "init");
        functions.emplace_back(bcastDeinit, "deinit");
        lua.createFunctionTable("bcast", functions);
        return 0;
    }
}
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

        vector<Message> messages = bm.receiveFromAll(5000);
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
