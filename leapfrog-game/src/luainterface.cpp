#include "luainterface.h"

#include "messagedisplay.h"
#include "objectproperty.h"

using namespace oxygine;

// Instansiate global class
LuaInterface g_LuaInterface;


// ------------------------------------------------------------------------
// General event handler for events handled by LUA script
void luaEventListner(oxygine::Event *ev)
{
   std::string actorName = ev->target->getName();
   std::string eventType = "1234";

   eventType[0] = (ev->type & 0xff);
   eventType[1] = ((ev->type >> 8) & 0xff);
   eventType[2] = ((ev->type >> 16) & 0xff);
   eventType[3] = ((ev->type >> 24) & 0xff);

   if (ev->type == eventID('O', 'p', 'T', 'r'))
   {
      // This is a Property trigger event, get some additional parameters
      ObjectPropertyTriggeredEvent* propTrigEv = static_cast<ObjectPropertyTriggeredEvent*>(ev);

      g_LuaInterface.missionStateSceneEventHandler(
         eventType, actorName, propTrigEv->m_propertyId);


   }
}



// ------------------------------------------------------------------------
// The functions we'll call from the lua script 
static int c_registerPropertyTrigger(lua_State *L)
{
   std::string actorName = lua_tostring(L, 1);
   int eventId = lua_tointeger(L, 2);
   int propertyIndex = lua_tointeger(L, 3);
   std::string triggerTypeStr = lua_tostring(L, 4);
   float lowerLimit = lua_tonumber(L, 5);
   float upperLimit = lua_tonumber(L, 6);
   
   PropertyEventTrigger::TriggerType triggerType;

   if (triggerTypeStr == "ignore")
   {
      triggerType = PropertyEventTrigger::TriggerType::ignore;
   }
   else if (triggerTypeStr == "insideRange")
   {
      triggerType = PropertyEventTrigger::TriggerType::insideRange;
   }
   else if (triggerTypeStr == "outsideRange")
   {
      triggerType = PropertyEventTrigger::TriggerType::outsideRange;
   }

   CompoundObject* object = g_LuaInterface.getSceneActor()->getObject(actorName);

   object->registerPropertyEventTrigger(propertyIndex, eventId, triggerType, lowerLimit, upperLimit);

   g_LuaInterface.registerTriggerHandle(object, propertyIndex, eventId);

   return 0;
}

static int c_registerEventHandler(lua_State *L)
{
   std::string actorName = lua_tostring(L, 1);
   std::string eventString = lua_tostring(L, 2);
   int eventId = lua_tointeger(L, 3);
   int propertyIndex = lua_tointeger(L, 4);

   oxygine::eventType eventType = eventID(
      eventString[0],
      eventString[1],
      eventString[2],
      eventString[3]);

   CompoundObject* actor = g_LuaInterface.getSceneActor()->getObject(actorName);

   int i = actor->addEventListener(eventType, luaEventListner);

   g_LuaInterface.registerHandlerHandle(actor, i);

   return 0;
}

static int c_clearAllTriggersAndEvents(lua_State *L)
{
   g_LuaInterface.clearAllTriggersAndEvents();
   return 0;
}

static int c_addMissionStateSceneObjects(lua_State *L)
{
   std::string missionStateSceneAdditionsFileName = lua_tostring(L, 1);

   SceneActor* scene = g_LuaInterface.getSceneActor();
   
   scene->addObjectsFromXmlFile(
      *scene->getResources(),
      scene,
      NULL,
      scene->getWorld(),
      missionStateSceneAdditionsFileName,
      *scene->getInitialState());
   
   return 0;
}

static int c_addDialogMessage(lua_State *L)
{
   std::string message = lua_tostring(L, 1);
   std::string sender = lua_tostring(L, 2);
   bool leftSide = lua_toboolean(L, 3);
   int preDelay = lua_tointeger(L, 4);
   int postDelay = lua_tointeger(L, 5);

   g_MessageDisplay->initMessage(leftSide, message.c_str(), sender.c_str());

   return 0;
}


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


// ------------------------------------------------------------------------
// The LUA interface implementation


LuaInterface::LuaInterface() : m_sceneActor(NULL)
{
}

void LuaInterface::initLuaInterface(void)
{
   // initialize Lua 
   m_L = luaL_newstate();
   luaL_openlibs(m_L);
//   lua_register(m_L, "average", average);

   ox::file::read("scene_navigator.lua", m_sceneNavigatorScriptBuffer);
   int result = luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_sceneNavigatorScriptBuffer[0]), m_sceneNavigatorScriptBuffer.size(), "scene_navigator");
   result = lua_pcall(m_L, 0, 0, 0);

   ox::file::read("mission_script.lua", m_sceneNavigatorScriptBuffer);
   result = luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_sceneNavigatorScriptBuffer[0]), m_sceneNavigatorScriptBuffer.size(), "mission_script");
   result = lua_pcall(m_L, 0, 0, 0);

   lua_register(m_L, "c_registerPropertyTrigger", c_registerPropertyTrigger);
   lua_register(m_L, "c_registerEventHandler", c_registerEventHandler);
   lua_register(m_L, "c_clearAllTriggersAndEvents", c_clearAllTriggersAndEvents);
   lua_register(m_L, "c_addMissionStateSceneObjects", c_addMissionStateSceneObjects);
   lua_register(m_L, "c_addDialogMessage", c_addDialogMessage);

   
}

//void LuaInterface::setSceneActor(SceneActor* sceneActor)
//{
//   m_sceneActor = sceneActor;
//}

SceneActor* LuaInterface::getSceneActor(void)
{
   return m_sceneActor;
}

void LuaInterface::registerHandlerHandle(oxygine::Actor* actor, int eventId)
{
   EventHandlerHandle handle(actor, eventId);
   m_eventHandlerHandles.push_back(handle);
}

void LuaInterface::registerTriggerHandle(CompoundObject* object, int propertyId, int eventId)
{
   TriggerHandle handle(object, propertyId, eventId);
   m_triggerHandles.push_back(handle);
}

void LuaInterface::clearAllTriggersAndEvents(void)
{
   for (auto it = m_eventHandlerHandles.begin(); it != m_eventHandlerHandles.end(); ++it)
   {
      it->m_actor->removeEventListener(it->m_eventId);
   }

   m_eventHandlerHandles.clear();

   for (auto it = m_triggerHandles.begin(); it != m_triggerHandles.end(); ++it)
   {
      it->m_object->getObjectProperty(it->m_propertyId)->unregisterPropertyEventTrigger(it->m_eventId);
   }

   m_triggerHandles.clear();
}

void LuaInterface::forceCurrentScene(const std::string& newCurrentScene)
{
   int a1 = lua_getglobal(m_L, "forceCurrentScene");
   lua_pushstring(m_L, newCurrentScene.c_str());
   lua_pcall(m_L, 1, 0, 0);
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

void LuaInterface::setupMissionStateScene(SceneActor* sceneActor)
{
   // This makes sure that the latest scene actor ís used, do not remove it
   m_sceneActor = sceneActor;

   int a1 = lua_getglobal(m_L, "setupMissionStateScene");
   int result = lua_pcall(m_L, 0, 0, 0);
}


void LuaInterface::missionStateSceneEventHandler(std::string eventId, std::string actorName, int parameter)
{
   int a1 = lua_getglobal(m_L, "missionStateSceneEventHandler");
   lua_pushstring(m_L, eventId.c_str());
   lua_pushstring(m_L, actorName.c_str());
   lua_pushinteger(m_L, parameter);
   int result = lua_pcall(m_L, 3, 0, 0);
}

void LuaInterface::runAverage(void)
{
   //luaL_loadbuffer(m_L, reinterpret_cast<char*>(&m_averageScriptBuffer[0]), m_averageScriptBuffer.size(), "test");
   //lua_call(m_L, 0, LUA_MULTRET);
}

