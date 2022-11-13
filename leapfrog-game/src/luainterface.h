#pragma once

#include "lua.hpp"
#include "oxygine-framework.h"

#include "sceneactor.h"
#include "mainactor.h"

// Some classes to remember registred events and triggers
// so that we can clean up after us
class EventHandlerHandle
{
public:
   oxygine::Actor* m_actor;
   int m_eventId;

   EventHandlerHandle(oxygine::Actor* actor, int eventId) :
      m_actor(actor),
      m_eventId(eventId)
   {   }
};

class TriggerHandle
{
public:
   CompoundObject* m_object;
   int m_propertyId;
   int m_eventId;

   TriggerHandle(CompoundObject* object, int propertyId, int eventId) :
      m_object(object),
      m_propertyId(propertyId),
      m_eventId(eventId)
   {   }
};

class LuaInterface
{
private:
   lua_State* m_L;
   ox::file::buffer m_sceneNavigatorScriptBuffer;
   SceneActor* m_sceneActor;
   MainActor* m_mainActor;
   std::vector<EventHandlerHandle> m_eventHandlerHandles;
   std::vector<TriggerHandle> m_triggerHandles;

public:
   LuaInterface();
   void initLuaInterface(MainActor* mainActor);
   //void setSceneActor(SceneActor* sceneActor);
   SceneActor* getSceneActor(void);
   MainActor* getMainActor(void);
   void registerHandlerHandle(oxygine::Actor* actor, int eventId);
   void registerTriggerHandle(CompoundObject* object, int propertyId, int eventId);
   void clearAllTriggersAndEvents(void);

   void lua_printPaths(void);
   bool isModulePreloaded(lua_State* L, std::string const& name);

   void lua_startInitialScene(void);
   void lua_forceCurrentScene(void);
   int lua_sceneExitHandler(SceneActor::SceneTypeEnum exitSceneType, int exitHow);
   int lua_findLeapfrogEntryPosition(SceneActor::SceneTypeEnum exitSceneType, int exitHow);

   void setupMissionStateScene(SceneActor* sceneActor);
   void setupInitialMissionStateScene(SceneActor* sceneActor);


   void missionStateSceneEventHandler(std::string eventId, std::string actorName, int parameter);

   void runAverage(void);

};

extern LuaInterface g_LuaInterface;