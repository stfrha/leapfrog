#pragma once

#include "lua.hpp"
#include "oxygine-framework.h"




class LuaInterface
{
private:
   lua_State* m_L;
   ox::file::buffer m_averageScriptBuffer;

public:
   LuaInterface();
   void initLuaInterface();
   void runAverage(void);

};

extern LuaInterface g_LuaInterface;