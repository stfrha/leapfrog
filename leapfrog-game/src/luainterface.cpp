#include "luainterface.h"

#include "messagedisplay.h"

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

   g_MessageDisplay->initMessage(true, "Hello, this is from a function called by a LUA script!", "LUA");

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
//   lua_register(m_L, "average", average);

   ox::file::read("scene_navigator.lua", m_sceneNavigatorScriptBuffer);

   lua_pushstring(m_L, "0.7");
   lua_setglobal(m_L, "VERSION");

   int result = luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_sceneNavigatorScriptBuffer[0]), m_sceneNavigatorScriptBuffer.size(), "sceneNavigatorScript");
   result = lua_pcall(m_L, 0, 0, 0);
}

int LuaInterface::determineNextScene(
   const std::string& entryType,
   const std::string& entryParameter,
   std::string& nextSceneFileName,
   std::string& nextSceneState,
   SceneActor::SceneTypeEnum& type)
{
   int a1 = lua_getglobal(m_L, "determineNextScene");
   lua_pushstring(m_L, entryType.c_str());
   lua_pushstring(m_L, entryParameter.c_str());

   // do the call (2 arguments, 2 result) 
   int result = lua_pcall(m_L, 2, 3, 0);

   if (result != 0)
   {
      // Something went wrong
      //return -1;
   }

   // retrieve result 
   if (!lua_isstring(m_L, -1))
   {
      // Wrong return type
      //return -2;
   }

   if (!lua_isstring(m_L, -2))
   {
      // Wrong return type
      //return -3;
   }

   std::string sceneType  = std::string(lua_tostring(m_L, -1));
   lua_pop(m_L, 1);  

   nextSceneState = std::string(lua_tostring(m_L, -1));
   lua_pop(m_L, 1);  

   nextSceneFileName = std::string(lua_tostring(m_L, -1));
   lua_pop(m_L, 1);  

   if (sceneType == "landing")
   {
      type = SceneActor::SceneTypeEnum::landing;
   }
   else if (sceneType == "deepSpace")
   {
      type = SceneActor::SceneTypeEnum::deepSpace;
   }
   else if (sceneType == "orbit")
   {
      type = SceneActor::SceneTypeEnum::orbit;
   }
   else if (sceneType == "hyperSpace")
   {
      type = SceneActor::SceneTypeEnum::hyperSpace;
   }


   return 0;

}


void LuaInterface::runAverage(void)
{
   //luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_averageScriptBuffer[0]), m_averageScriptBuffer.size(), "test");
   //lua_call(m_L, 0, LUA_MULTRET);
}

