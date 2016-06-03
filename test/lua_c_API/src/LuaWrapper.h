#ifndef LuaWrapper_INCLUDE_GUARD
#define LuaWrapper_INCLUDE_GUARD

extern "C" 
{
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#include <exception>
#include <string>
#include <vector>

struct LuaCFunction
{
    lua_CFunction address;
    std::string name;

    LuaCFunction(lua_CFunction addr,
                 std::string functionName)
        :address(addr),
         name(functionName)
    {}
};

/*
 * Class that represents the lua interpreter
 */
class LuaInterpreter
{
private:
   lua_State* luaState_;
   bool cleanState_;
public:
   //Constructs a new lua state
   //and cleans it when the object
   //goes out of scope.
   LuaInterpreter();
   //The lua state is received from
   //the outside. The clean flag
   //specifies if the state should
   //be cleaned by the destructor or not.
   LuaInterpreter(lua_State* luaState,
                  bool clean = false);
   //remove copy and move operations
   LuaInterpreter(const LuaInterpreter& source) = delete;
   LuaInterpreter& operator=(const LuaInterpreter& source) = delete;
   LuaInterpreter(LuaInterpreter&& source) = delete;
   LuaInterpreter& operator=(LuaInterpreter&& source) = delete;
   //destructor
   ~LuaInterpreter();

   //Creates a table with the given name
   //and adds all the functions from the
   //vector to it.
   void createFunctionTable(std::string tableName,
                            std::vector<LuaCFunction> functions);
};

class LuaException : public std::exception
{
private:
    std::string message_;
public:
    LuaException(std::string message)
        :message_(message)
    {}

    const char* what()const noexcept override
    {
        return message_.c_str();
    }
};
#endif
