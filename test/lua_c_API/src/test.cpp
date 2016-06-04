extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LuaWrapper.h"

using std::vector;

struct Adder
{
    int state;
    
    Adder(int argState)
        :state(argState)
    {}

    int getState()
    {
        return state;
    }

    void addState(int s)
    {
        state += s;
    }
};

static Adder g_adder(0);

int getAdder(lua_State* L)
{
    int state = g_adder.getState();

    lua_pushnumber(L, state);

    return 1;
}

int add(lua_State* L)
{
    LuaInterpreter lua(L);
    try
    {
        double number = lua.getNumber(1);
        g_adder.addState(number);
        return 0;
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
        return 1;
    }
}

int getMessages(lua_State* L)
{
    vector<TestMessage> messages;
    messages.emplace_back("id1", "Hello");
    messages.emplace_back("id2", "world");

    LuaInterpreter lua(L);
    lua.pushMessages(messages);

    return 1;
}

extern "C" {
    int luaopen_testLib(lua_State *L)
    {
        LuaInterpreter li(L);

        std::vector<LuaCFunction> functions;
        functions.emplace_back(getAdder, "get");
        functions.emplace_back(add, "add");
        functions.emplace_back(getMessages, "getMessages");
        li.createFunctionTable("testLib", functions);
        return 0;
    }
}
