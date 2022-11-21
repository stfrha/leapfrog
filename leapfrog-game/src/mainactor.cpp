
#include <functional>
#include "mainactor.h"

#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include "physdispconvert.h"
#include "scales.h" 
#include "compoundobjectevents.h"
#include "landingactorevents.h"
#include "deepspacesceneevents.h"
#include "layout.h"
#include "gamestatus.h"
#include "statusbar.h"
#include "orbitscene.h"
#include "orbitspacescene.h"
#include "headdowndisplay.h"
#include "headupdisplay.h"
#include "messagedisplay.h"
#include "luainterface.h"
#include "menuactor.h"
#include "graphicresources.h"

#include "objectresourcesevents.h"

using namespace oxygine;
using namespace std;

NextSceneDefinition::NextSceneDefinition() :
   m_nextSceneFile("landing_scene.xml"),
   m_armNextScene(true),
   m_nextSceneType(SceneActor::SceneTypeEnum::landing)
{
}


// Instanciate global instance of the Main actor
spMainActor g_MainActor;

MainActor::MainActor() :
   m_world(NULL),
   m_splashArm(true),
   m_reloadPressed(false),
   m_reloadArm(true),
   m_menuArm(true),
   m_sceneObject(NULL)
{
   m_splashScreenResource.loadXML("splash_screen.xml");

   // Now center splash on center of screen
   oxygine::Sprite* splashSprite = new  oxygine::Sprite();
   splashSprite->setResAnim(m_splashScreenResource.getResAnim("splash_screen"));

   // Determine sprite size so it fills the screen

   //logs::messageln("Stage width %f, Stage Height: %f", (getStage()->getSize()).x, (getStage()->getSize()).y);
   //logs::messageln("Sprite width %f, Sprite Height: %f", splashSprite->getWidth(), splashSprite->getHeight());

   float hr = (getStage()->getSize()).y / splashSprite->getHeight();
   float wr = (getStage()->getSize()).x / splashSprite->getWidth();
   float rate = 0.0f;
   if (hr > wr)
   {
      rate = hr;
   }
   else
   {
      rate = wr;
   }

   splashSprite->setSize(splashSprite->getWidth() * rate, splashSprite->getHeight() * rate);

   // logs::messageln("New Sprite width %f, New Sprite Height: %f", splashSprite->getWidth(), splashSprite->getHeight());

   splashSprite->setAnchor(Vector2(0.5f, 0.5f));
   splashSprite->setPosition((getStage()->getSize()).x / 2, (getStage()->getSize()).y / 2);
   splashSprite->attachTo(this);
}

void MainActor::initMainActor(void)
{
   m_splashScreenResource.unload();

   //load xml file with resources definition
   g_GraphRes.loadResource(GraphicResources::ResourceTypeEnum::game, "res.xml");
   g_GraphRes.loadResource(GraphicResources::ResourceTypeEnum::hud, "res_hud.xml");

   g_LuaInterface.initLuaInterface();

   g_LuaInterface.lua_printPaths();

   g_HeadDownDisplay = new HeadDownDisplay();
   g_MessageDisplay = new MessageDisplay();
   g_headUpDisplay = new HeadUpDisplay();

   g_MessageDisplay->initialiseMessageDisplay(
      getStage().get(), 
      0.0f, 
      g_Layout.getButtonWidth(), 
      0.0f, 
      g_Layout.getButtonWidth() * 2.0f, 
      g_Layout.getViewPortBounds().y, 
      g_Layout.getYFromBottom(1) - g_Layout.getButtonWidth() / 2.0f,
      g_Layout.getDefaultFontSize());

   setSize(g_Layout.getViewPortBounds());


   // restore previous state and continue from there
   g_GameStatus.restoreGameStatus();

   if (g_GameStatus.getExitSceneType() == -1)
   {
      // Exit scene data is NOT available, run initial position for leapfrog
      g_LuaInterface.lua_startInitialScene();
   }
   else
   {
      // Exit scene data is available, use that
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_findLeapfrogEntryPosition(
         static_cast<SceneActor::SceneTypeEnum>(
            g_GameStatus.getExitSceneType()),
         g_GameStatus.getExitParameter());

      armScene(g_GameStatus.getCurrentScene(), g_GameStatus.getCurrentSceneType());
   }


}


MainActor::~MainActor()
{
}


void MainActor::armScene(std::string sceneName, int sceneType)
{
   m_nextScene.m_nextSceneFile = sceneName;
   m_nextScene.m_nextSceneType = static_cast<SceneActor::SceneTypeEnum>(sceneType);
   m_nextScene.m_armNextScene = true;
}

void MainActor::startScene(void)
{
   g_MessageDisplay->cleanAndClearMessageDisplay();

   // Remove listners which includes scene transit events
   removeAllEventListeners();

   // Clear all triggers and events from the current
   // actors before they are destroyed.
   g_LuaInterface.clearAllTriggersAndEvents();

   // Clean up current scene
   spActor actor = getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
         recursiveRemoveChildren(actor);
         actor->detach();
      }
      actor = next;
   }

   g_HeadDownDisplay->clearMap();

   if (m_world != NULL)
   {
      delete m_world;
      m_world = NULL;
   }

   // Gravity resets to zero. The Scene behaviour must set the gravity
   // by itself
   m_world = new b2World(b2Vec2(0.0f, 0.0f));

   spClipRectActor window = new ClipRectActor();

   window->setSize(g_Layout.getViewPortBounds());
   window->setPosition(0.0f, 0.0f);
   addChild(window);

   if ((m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::landing) ||
      (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::deepSpace))
   {
      spSceneActor sceneObj = static_cast<SceneActor*>(CompoundObject::readDefinitionXmlFile(
            NULL,
            NULL,
            m_world,
            Vector2(0.0f, 0.0f),
            m_nextScene.m_nextSceneFile));

      if (sceneObj == NULL)
      {
         return;
      }

      window->addChild(sceneObj);

      if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::landing)
      {
         m_sceneObject = sceneObj.get();
         sceneObj->addEventListener(ExitLandingSceneEvent::EVENT, CLOSURE(this, &MainActor::exitLandingScene));
      }
      else
      {
         m_sceneObject = sceneObj.get();
         sceneObj->addEventListener(ExitDeepSpaceSceneEvent::EVENT, CLOSURE(this, &MainActor::exitDeepSpaceScene));
      }
   }
   else if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit)
   {
      spCompoundObject co = static_cast<SceneActor*>(CompoundObject::readDefinitionXmlFile(
            NULL,
            NULL,
            m_world,
            Vector2(0.0f, 0.0f),
            m_nextScene.m_nextSceneFile));

      window->addChild(co);

      OrbitScene* os = static_cast<OrbitScene*>(co.get());
      m_sceneObject = os->m_space;

      co.get()->addEventListener(ExitOrbitSceneEvent::EVENT, CLOSURE(this, &MainActor::exitOrbitScene));
   }

   addEventListener(StatusResourceDepletedEvent::EVENT, CLOSURE(this, &MainActor::resourceDepletedHandler));

   g_HeadDownDisplay->initialiseHdd(
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize());

   g_headUpDisplay->initialiseHeadUpDisplay(
      m_sceneObject,
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize()); 

   if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit)
   {
      g_HeadDownDisplay->goToOrbit();
      g_headUpDisplay->showStatusBars(false);
   }
   else
   {
      g_HeadDownDisplay->goToMap();
   }

   g_LuaInterface.setupMissionStateScene(m_sceneObject);

   m_sceneObject->connectToForeignObjects();

   // Now all is initiated. If this is the initial mission scene we need to 
   // save the game state to say that the first mission has begun. 
   if (g_GameStatus.getExitSceneType() == -1)
   {
      g_GameStatus.saveGameStatus();
   }

   //m_debugDraw = new Box2DDraw;
   //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
   //m_debugDraw->attachTo(m_sceneObject);
   //m_debugDraw->setWorld(Scales::c_physToStageScale, m_sceneObject->GetWorld());
   //m_debugDraw->setPriority(2550);
}

float MainActor::getProperty(std::string object, int propId)
{
   CompoundObject* co = m_sceneObject->getObject(object);
   if (co)
   {
      return co->getProperty(propId);
   }
   return -FLT_MAX;
}

void MainActor::setProperty(std::string object, int propId, float value)
{
   CompoundObject* co = m_sceneObject->getObject(object);
   if (co)
   {
      co->setProperty(propId, value);
   }
}

void MainActor::registerPropertyTrigger(
   std::string object,
   int propId,
   int eventId,
   PropertyEventTrigger::TriggerType type,
   float lowerLimit,
   float upperLimit)
{
   CompoundObject* co = m_sceneObject->getObject(object);
   if (co)
   {
      co->registerPropertyEventTrigger(eventId, propId, type, lowerLimit, upperLimit);
   }
}

void MainActor::registerDualPropTrigger(
   std::string object,
   int propId,
   DualPropEventTrigger* trigger)
{
   CompoundObject* co = m_sceneObject->getObject(object);
   if (co)
   {
      co->registerDualPropEventTrigger(propId, trigger);
   }
}

void MainActor::unregisterDualPropTrigger(
   std::string object,
   int eventId)
{

}

void MainActor::exitLandingScene(oxygine::Event *ev)
{
   ExitLandingSceneEvent* xev = static_cast<ExitLandingSceneEvent*>(ev);
   g_LuaInterface.lua_sceneExitHandler(SceneActor::SceneTypeEnum::landing, xev->m_parameter);
}

void MainActor::exitDeepSpaceScene(oxygine::Event *ev)
{
   ExitDeepSpaceSceneEvent* xev = static_cast<ExitDeepSpaceSceneEvent*>(ev);
   g_LuaInterface.lua_sceneExitHandler(SceneActor::SceneTypeEnum::deepSpace, xev->m_parameter);
}

void MainActor::exitOrbitScene(oxygine::Event *ev)
{
   ExitOrbitSceneEvent* xev = static_cast<ExitOrbitSceneEvent*>(ev);
   g_LuaInterface.lua_sceneExitHandler(SceneActor::SceneTypeEnum::orbit, xev->m_parameter);
}

void MainActor::exitHyperspaceScene(oxygine::Event *ev)
{

}

void MainActor::resourceDepletedHandler(oxygine::Event *ev)
{
   StatusResourceDepletedEvent* e = (StatusResourceDepletedEvent*)ev;


   switch (e->m_type)
   {
   case fuel:
      // Stop current burning 
      if (m_sceneObject->m_leapfrog != NULL)
      {
         m_sceneObject->m_leapfrog->m_boosterFlame->stopEmitter();
         m_sceneObject->m_leapfrog->m_leftSteerFlame->stopEmitter();
         m_sceneObject->m_leapfrog->m_rightSteerFlame->stopEmitter();
      }

      break;
   case credits:
      break;
   case shots:
      break;
   case shield:
      break;
   case damage:
      break;
   }
}

void MainActor::fetchInternetScene(void)
{
   ox::HttpRequestTask::init();

   ox::spHttpRequestTask task = ox::HttpRequestTask::create();
   task->setUrl("http://www.fredrikhoffman.se/leapfrog/updated_scene.xml");
   task->setFileName("updated_scene.xml");
   task->addEventListener(HttpRequestTask::COMPLETE, CLOSURE(this, &MainActor::httpLoaded));
   task->run();

   //ox::HttpRequestTask::release();
}

void MainActor::httpLoaded(Event* event)
{
   ox::HttpRequestTask* task = safeCast<ox::HttpRequestTask*>(event->currentTarget.get());

   //convert it to string and print
   //std::string resp;
   //resp.assign(response.begin(), response.end());

   //ox::logs::messageln("server response: %s", resp.c_str());

   // TODO: For now, this is hardcoded, but somhow, it needs to be configurable
   m_nextScene.m_nextSceneType = SceneActor::SceneTypeEnum::landing;
   m_nextScene.m_armNextScene = true;
   m_nextScene.m_nextSceneFile = "updated_scene.xml";

   ox::HttpRequestTask::release();


}

void MainActor::doUpdate(const UpdateState& us)
{
   if (m_splashArm)
   {
      m_splashArm = false;
      initMainActor();
   }

   if ((m_sceneObject != NULL) && (m_sceneObject->getIsInPause()) && (m_menuArm == true))
   {
      // g_headUpDisplay->cleanAndRemove();
      g_headUpDisplay->showStatusBars(false);
      g_headUpDisplay->setTouchEnabled(false);
      g_headUpDisplay->showButtons(false);
      g_HeadDownDisplay->goToMenu();
      g_MessageDisplay->setFullHeight(true);

      m_menuArm = false;

   }

   if (m_reloadArm && m_reloadPressed)
   {
      m_reloadArm = false;
   }

   if (!m_reloadArm && !m_reloadPressed)
   {
      m_reloadArm = true;

      // Load scene from internet
      fetchInternetScene();

      return;
   }

   if (m_nextScene.m_armNextScene)
   {
      m_nextScene.m_armNextScene = false;

      startScene();

   }
   else
   {

   }
}



void MainActor::recursiveRemoveChildren(spActor& parent)
{
   spActor child = parent->getFirstChild();

   while (child != NULL)
   {
      spActor next = child->getNextSibling();

      if (child.get() != NULL)
      {
         logs::messageln("Removing children from parent: %s", child->getName().c_str()/*, typeid(child.get()).name()*/);
         recursiveRemoveChildren(child);

         logs::messageln("Removing child: %s" , child->getName().c_str()/*, typeid(child.get()).name()*/);

         if (child->getName() == "leapfrog1")
         {
            int a = 10;
         }

         child->detach();
      }


      child = next;
   }
}

void MainActor::buttonClicked(int i)
{
   if (i == 1)
   {
      m_postMenuAction = PostMenuActionType::continueCurrent;

      if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit)
      {
         g_HeadDownDisplay->goToOrbit();
      }
      else
      {
         g_HeadDownDisplay->goToMap();
      }
   }
   else if (i == 2)
   {
      m_postMenuAction = PostMenuActionType::continueLatest;
      g_HeadDownDisplay->goToMap();
   }
   else if (i == 3)
   {
      m_postMenuAction = PostMenuActionType::startNew;
      g_HeadDownDisplay->goToMap();
   }
   else if (i == 4)
   {
      m_postMenuAction = PostMenuActionType::testLanding;
      g_HeadDownDisplay->goToMap();
   }
   else if (i == 5)
   {
      m_postMenuAction = PostMenuActionType::testSpace;
      g_HeadDownDisplay->goToMap();
   }
   else if (i == 6)
   {
      m_postMenuAction = PostMenuActionType::testOrbit;
      g_HeadDownDisplay->goToMap();
   }
}


void MainActor::restartedFromMenu(void)
{
   m_menuArm = true;
   m_sceneObject->setIsInPause(false);

   g_MessageDisplay->setFullHeight(false);

   if (m_postMenuAction == PostMenuActionType::continueCurrent)
   {
      // Release pause
      g_headUpDisplay->setTouchEnabled(true);
      g_headUpDisplay->showButtons(true);
      g_headUpDisplay->showStatusBars(m_nextScene.m_nextSceneType != SceneActor::SceneTypeEnum::orbit);
      //g_headUpDisplay->initialiseHeadUpDisplay(
      //   this,
      //   m_sceneObject,
      //   Vector2(0.0f, 0.0f),
      //   m_sceneObject->getSize());
      // g_headUpDisplay->registerLeapfrog(m_sceneObject->m_leapfrog, (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit));

      //g_HeadDownDisplay->initialiseHdd(
      //   this,
      //   Vector2(0.0f, 0.0f),
      //   m_sceneObject->getSize());



      //if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit)
      //{
      //   g_HeadDownDisplay->goToOrbit();
      //}
      //else
      //{
      //   g_HeadDownDisplay->goToMap();
      //}
   }
   else if (m_postMenuAction == PostMenuActionType::startNew)
   {
      // Initialise new game data and load first scene according to lua
      g_GameStatus.initializeGameStatusXmlDocument();
      g_GameStatus.gameStatusNewGameInit();
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_startInitialScene();
   }
   else if (m_postMenuAction == PostMenuActionType::continueLatest)
   {
      // restore previous state and continue from there
      g_GameStatus.restoreGameStatus();

      if (g_GameStatus.getExitSceneType() == -1)
      {
         // Exit scene data is NOT available, run initial position for leapfrog
         g_LuaInterface.lua_startInitialScene();
      }
      else
      {
         // Exit scene data is available, use that
         g_LuaInterface.lua_forceCurrentScene();
         g_LuaInterface.lua_findLeapfrogEntryPosition(static_cast<SceneActor::SceneTypeEnum>(g_GameStatus.getExitSceneType()),
            g_GameStatus.getExitParameter());

         armScene(g_GameStatus.getCurrentScene(), g_GameStatus.getCurrentSceneType());
      }
   }
   else if (m_postMenuAction == PostMenuActionType::testLanding)
   {
      g_GameStatus.setSceneMissionState("landing_scene.xml", 1, 1, 0, "orbit_scene.xml", 2, 0);
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_startInitialScene();
   }
   else if (m_postMenuAction == PostMenuActionType::testSpace)
   {
      g_GameStatus.setSceneMissionState("deep_space_scene.xml", 1, 1, 1, "landing_scene.xml", 0, 0);
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_startInitialScene();
   }
   else if (m_postMenuAction == PostMenuActionType::testOrbit)
   {
      g_GameStatus.setSceneMissionState("orbit_scene.xml", 1, 1, 2, "deep_space_scene.xml", 1, 0);
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_startInitialScene();
   }
}

void MainActor::menuStartTransitionComplete(void)
{
   spMenuActor ma = new MenuActor(
      this);
}

void MainActor::cleanUpAndQuit(void)
{
   // Remove listners which includes scene transit events
   removeAllEventListeners();

   // Clear all triggers and events from the current
   // actors before they are destroyed.
   g_LuaInterface.clearAllTriggersAndEvents();


   // Clean up current scene
   spActor actor = getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
         // logs::messageln("Removing children from parent: %s", actor->getName().c_str()/*, typeid(child.get()).name()*/);
         recursiveRemoveChildren(actor);

         // logs::messageln("Removing child: %s", actor->getName().c_str() /*, typeid(actor.get()).name()*/);

         actor->detach();
      }
      actor = next;
   }

   core::requestQuit();
}