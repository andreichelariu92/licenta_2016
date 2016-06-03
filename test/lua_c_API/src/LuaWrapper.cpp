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
