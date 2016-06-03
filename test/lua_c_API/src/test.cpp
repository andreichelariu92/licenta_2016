extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LuaWrapper.h"

int customPrint(lua_State* L)
{
    const char* message = lua_tostring(L, 1);
    printf("from managed side %s\n", message);
    return 0;
}
extern "C" {
int luaopen_testLib(lua_State *L)
{
    LuaInterpreter li(L);

    std::vector<LuaCFunction> functions;
    functions.emplace_back(customPrint, "customPrint");
    li.createFunctionTable("testLib", functions);
    return 1;
}
}
