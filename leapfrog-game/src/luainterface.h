#pragma once

#include "lua.hpp"
#include "oxygine-framework.h"

#include "sceneactor.h"




class LuaInterface
{
private:
   lua_State* m_L;
   ox::file::buffer m_sceneNavigatorScriptBuffer;

public:
   LuaInterface();
   void initLuaInterface();
   void forceCurrentScene(const std::string& newCurrentScene);
   int determineNextScene(
      const std::string& entryType,
      const std::string& entryParameter,
      std::string& nextSceneFileName,
      std::string& nextSceneState,
      SceneActor::SceneTypeEnum& type);

   void runAverage(void);

};

extern LuaInterface g_LuaInterface;