#include "luainterface.h"

#include "messagedisplay.h"
#include "objectproperty.h"

using namespace oxygine;

// Instansiate global class
LuaInterface g_LuaInterface;


// How to handle Object Property Trigger events:
//
// The initiator is the LUA script. It uses the c_registerPropertyTrigger
// method to register a property trigger. To this, it supplies the following:
// - The actor name of the object to monitor
// - The event id (TODO: Is this a general number or what?)
// - The id (or index) of the property for the object type of the actor name
// - The type of trigger (inside, outside, etc.)
// - The range limits (always two even if only one is used)
// The object actor is fetched from the scene and the event trigger 
// is registred in it, with all information so that is can
// launch a "OpTr"-event (Object Property Trigger) when the 
// condition is defined. The actor method: registerPropertyEventTrigger 
// is used for this. In the c_registerPropertyTrigger method
// a handle to the trigger is generated. This holds the following:
// - A pointer to the actor
// - The id (or index) of the property for the object type of the actor name
// - The event id (TODO: Is this a general number or what?)
// This is stored in the m_triggerHandles and is used to clean up the trigger
// from the actor.
// The LUA-script now needs to register that it wants to handle
// the OpTr event.
// This is done with the c_registerEventHandler method. To this function 
// the following data is supplied from the script:
// - The actor name of the object with the property that will trigger the event
// - The event type (four characters)
// - The event id (TODO: Is this a general number or what?)
// - The id (or index) of the property for the object type of the actor name
// The object actor is fetched from the scene and the event listner is registrated
// in it.
// The event handler is always the luaEventListner method. The registration
// returns with an id for this event handling. 
// Then the event is registred in the lua interface. This is done in the 
// registerHandlerHandle method. To this method the following is added:
// - A pointer to the actor object
// - The event handler id returned from the handler registration in the actor.
// Using the supplied data, a TriggerHandle is created and pushed into a list
// of such handles: m_eventHandlerHandles.
//
// The registration is now complete. When the property satisfies the defined trigger
// condition, the following happens:
// The object that has the property will evaluate the property value each time it is 
// changed. It has a list of triggers for this property that each may have individual
// limits. Each trigger is iterated. If there is a match in the value and the trigger
// the actor sends the OpTr-event.
// The event is handled by the registrated handler, which, acc. to above, is the
// luaEventListner. In this, the following information is extracted from the event:
// - the name of the actor that sent the event
// - The event type (four characters)
// If the event was cased by a Object Property Trigger (== "OpTr"), than
// an additional parameter is extracted: 
// - Property identity (sometimes called Property Index)
// This information is sent to the LUA-interface using the LUA function:
// missionStateSceneEventHandler. In the LUA-script the supplied data 
// is decoded with if-statements. Is it from THIS actor, is a OpTr event,
// is it THIS property identity, are we in THIS state: In that case, do THIS.
// Can be go to another state, play a message, load additional sceene objects, etc.
// Thats is it.
// During this sequence, data is stored in a number of lists:
// 1. actor.m_properties[property id].m_eventTriggers gets a event trigger
// 2. m_triggerHandles get a handle to the trigger in the actor
// 3. actor._listners gets the new event listner (this is the oxygine internal event handler)
// 4. m_eventHandlerHandles get a handle to the event handler in the actor
//
// Clean-up
// Clean up is done by the LUA-script when needed. Basically, it will only execute at 
// event trigger, so there will always be caused by an event, usually when the state of the 
// mission is changed. At this time all triggers and related events are removed. 
// This is done by the script by calling the c_clearAllTriggersAndEvents method.
// In this all the event handler handles in the m_eventHandlerHandles are iterated
// and the event handler is unregistrated from the actor. The m_eventHandlerHandles
// list is then cleared.
// The m_triggerHandles list is then iterated and all triggers are unregistrated from
// the actor's property using the unregisterPropertyEventTrigger method in the property.
// The m_triggerHandles list is then cleared. Clean-up is complete. 
// The LUA-script can now set the next state and determine what needs to be done for the 
// new state in the current scene, which involves setting up new triggers and event handlers.
// 
// New scenes:
// When the mainactor loads a new scene, in the startScene method, it will execute the 
// LUA-script function setupMissionStateScene. Here all triggers and events are 
// registrated. Normally, the cleanup is done by the script, when state is changeed.
// The script then calls the c_clearAllTriggersAndEvents method.
//
// What happens when a scene is changed because the player launched itself into space?
// In the startScene method, an event listner is registrated in the scene object that
// is triggered by the scene transit.
// In this handler, in the mainactor, the LUA-script function: determineNextScene is called.
// This function uses global LUA-variables that holds the what type of scene the player came
// from and the scene identity that is came from and using this is sets the nextScene, sceneType
// and sceneState (which is not the same as the mission state). This function does no registration
// or cleaning of events or triggers. The mainactor then arms the scene change which means to
// execute the scene change (call startScene) the next doUpdate.
// In the startScene all triggers and events stored in the LUA-interface is cleared while the
// associated actors are still alive. They are about to be removed. Then all event listners 
// of the mainactor is removed (including touch events and the scene tranist events). 
// Then the previous scene is cleard of actors. 
// Now, new event handlers are re-created for the touch-events.
// Then the new scene objects are created from the XML-files and new scene transit events are
// registrated.
// Then the LUA-script function: setupMissionStateScene is called.
// 




// ------------------------------------------------------------------------
// General event handler for events handled by LUA script. From the event, 
// the following information is extracted:
// - the name of the actor that sent the event
// - The event type (four characters)
// If the event was cased by a Object Property Trigger (== "OpTr"), than
// an additional parameter is extracted: 
// - Property identity (sometimes called Property Index)
// 
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
   else
   {
      g_LuaInterface.missionStateSceneEventHandler(
         eventType, actorName, 0);
   }
}



// ------------------------------------------------------------------------
// The functions we'll call from the lua script 
// ------------------------------------------------------------------------




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



// This function is called by the LUA-scripts to register a handler
// for an event. The LUA interface has one single handler that receives
// all events: luaEventListner. This triggers a LUA-script function
// that decodes the event data to determine what to do. 
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

static int c_setPanningObject(lua_State *L)
{
   std::string objectName = lua_tostring(L, 1);

   SceneActor* scene = g_LuaInterface.getSceneActor();

   scene->setPanorateObject(scene->getObject(objectName));

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
   lua_register(m_L, "c_setPanningObject", c_setPanningObject);

   
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

