#include "LuaWrapper.h"

using std::vector;
using std::string;

LuaInterpreter::LuaInterpreter()
    :luaState_(luaL_newstate()),
     cleanState_(true)
{
    if (luaState_ == NULL)
    {
        LuaException e("cannot create interpreter");
        throw e;
    }
}

LuaInterpreter::LuaInterpreter(lua_State* luaState,
                               bool clean)
    :luaState_(luaState),
     cleanState_(clean)
{
    if (luaState_ == NULL)
    {
        LuaException e("bad interpreter given");
        throw e;
    }
}

LuaInterpreter::~LuaInterpreter()
{
    if (cleanState_)
    {
        lua_close(luaState_);
    }
}

void LuaInterpreter::createFunctionTable(string tableName,
                                         vector<LuaCFunction> funcs)
{
    //add new table on the top
    //of the stack
    lua_newtable(luaState_);

    for (LuaCFunction& func : funcs)
    {
        //add function name
        lua_pushstring(luaState_, func.name.c_str());
        //add function address
        lua_pushcfunction(luaState_, func.address);
        //add key, value pair to the table
        lua_settable(luaState_, -3);
    }

    //give the table name
    lua_setglobal(luaState_, tableName.c_str());
}

double LuaInterpreter::getNumber(int index)
{
    int isNumber;
    double result = lua_tonumberx(luaState_, index, &isNumber);

    if (!isNumber)
    {
        LuaException e("not a number at index " + index);
        throw e;
    }

    return result;
}

string LuaInterpreter::getString(int index)
{
    size_t stringLen = 0;
    const char* stringPtr = 
        lua_tolstring(luaState_, index, &stringLen);

    if (stringPtr == NULL) {
        LuaException e("not a string at index " + index);
        throw e;
    }

    return std::string(stringPtr, stringLen);
}

void LuaInterpreter::pushMessage(string messageId,
                                 string messageBuffer)
{
    //create table that will hold the
    //message
    lua_newtable(luaState_);

    //add messageId
    lua_pushstring(luaState_, "id");
    lua_pushstring(luaState_, messageId.c_str());
    lua_settable(luaState_, -3);

    //add messageBuffer
    lua_pushstring(luaState_, "buffer");
    lua_pushstring(luaState_, messageBuffer.c_str());
    lua_settable(luaState_, -3);
}

void LuaInterpreter::pushMessages(vector<TestMessage> messages)
{
    //create the table that will
    //hold all the messages
    lua_newtable(luaState_);
    
    int messageIdx = 1;//lua start indexes from 1
    for (TestMessage& message : messages)
    {
        //push index
        lua_pushnumber(luaState_, messageIdx);
        //push the message
        pushMessage(message.messageId, message.messageBuffer);
        //add the message to the list
        lua_settable(luaState_, -3);

        ++messageIdx;
    }
}
