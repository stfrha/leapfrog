#include "luainterface.h"
#include "oxygine-framework.h"

using namespace oxygine;

// Instansiate global class
LuaInterface g_LuaInterface;

/* The function we'll call from the lua script */
static int average(lua_State *L)
{
   /* get number of arguments */
   int n = lua_gettop(L);
   double sum = 0;
   int i;
   
   /* loop through each argument */
   for (i = 1; i <= n; i++)
   {
      if (!lua_isnumber(L, i))
      {
         lua_pushstring(L, "Incorrect argument to 'average'");
         lua_error(L);
      }

      /* total the arguments */
      sum += lua_tonumber(L, i);
   }

   /* push the average */
   lua_pushnumber(L, sum / n);

   /* push the sum */
   lua_pushnumber(L, sum);

   /* return the number of results */
   return 2;
}

LuaInterface::LuaInterface()
{
}

void LuaInterface::initLuaInterface(void)
{
   // initialize Lua 
   m_L = luaL_newstate();
   luaL_openlibs(m_L);
   lua_register(m_L, "average", average);

   ox::file::read("average.lua", m_averageScriptBuffer);

   lua_pushstring(m_L, "0.7");
   lua_setglobal(m_L, "VERSION");
}

void LuaInterface::runAverage(void)
{
   luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_averageScriptBuffer[0]), m_averageScriptBuffer.size(), "test");
   lua_call(m_L, 0, LUA_MULTRET);
}

