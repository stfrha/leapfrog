
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
   m_fpsEstArm(false),
   m_splashArm(true),
   m_reloadPressed(false),
   m_reloadArm(true),
   m_menuArm(true),
   m_initArm(false),
   m_sceneObject(NULL),
   m_isInPause(false),
   m_isInMessagePause(false),
   m_testStartIndex(1),
   m_startingTest(false),
   m_fpsEst(NULL)
{
}

void MainActor::showSplashScreen(void)
{
   m_splashScreenResource.loadXML("splash_screen.xml");

   // Now center splash on center of screen
   spSprite splashSprite = new  oxygine::Sprite();
   splashSprite->setResAnim(m_splashScreenResource.getResAnim("splash_screen"));

   // Determine sprite size so it fills the screen

   logs::messageln("Stage width %f, Stage Height: %f", (getStage()->getSize()).x, (getStage()->getSize()).y);
   logs::messageln("Sprite width %f, Sprite Height: %f", splashSprite->getWidth(), splashSprite->getHeight());

   float hr = (getStage()->getSize()).y / splashSprite->getHeight();
   float wr = (getStage()->getSize()).x / splashSprite->getWidth();
   float rate = 0.0f;
   if (hr < wr)
   {
      rate = hr;
   }
   else
   {
      rate = wr;
   }

   splashSprite->setSize(splashSprite->getWidth() * rate, splashSprite->getHeight() * rate);

   logs::messageln("New Sprite width %f, New Sprite Height: %f", splashSprite->getWidth(), splashSprite->getHeight());

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
      g_Layout.getButtonWidth() * 0.1f,   // normal top: nothing above it on the left
      g_Layout.getXFromLeft(0), 
      g_Layout.getXFromLeft(2), 
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
   // Messages shown so far stay; those not yet shown are dropped
   g_MessageDisplay->newScene();

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

   // The old scene's actors are gone; switch to the new scene's own
   // resource set (if it has one) before its objects look up textures.
   g_GraphRes.loadSceneResources(m_nextScene.m_nextSceneFile);

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

      m_clock = createSceneClock();
      sceneObj->setClock(m_clock);

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
      spOrbitScene sceneObj = static_cast<OrbitScene*>(CompoundObject::readDefinitionXmlFile(
            NULL,
            NULL,
            m_world,
            Vector2(0.0f, 0.0f),
            m_nextScene.m_nextSceneFile));

      window->addChild(sceneObj);

      m_clock = createSceneClock();
      sceneObj->setClock(m_clock);

      m_sceneObject = sceneObj->m_space;

      sceneObj->addEventListener(ExitOrbitSceneEvent::EVENT, CLOSURE(this, &MainActor::exitOrbitScene));
   }

   addEventListener(StatusResourceDepletedEvent::EVENT, CLOSURE(this, &MainActor::resourceDepletedHandler));

   g_HeadDownDisplay->initialiseHdd(
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize());

   g_headUpDisplay->initialiseHeadUpDisplay(
      m_sceneObject,
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize()); 

   g_headUpDisplay->setTouchEnabled(true);

   if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::orbit)
   {
      g_HeadDownDisplay->goToOrbit();
      g_headUpDisplay->showStatusBars(false);

      // The camera pan button is for debugging landing and deep space
      // scenes; the orbit display needs its place
      g_headUpDisplay->showPanButton(false);
   }
   else
   {
      g_HeadDownDisplay->goToMap();
   }

   placeMessageDisplay();

   g_LuaInterface.setupMissionStateScene(m_sceneObject);

   m_sceneObject->connectToForeignObjects();

   // Now all is initiated. If this is the initial mission scene we need to 
   // save the game state to say that the first mission has begun. 
   // (Not for a test start: it leaves the saved game as it was)
   if ((g_GameStatus.getExitSceneType() == -1) && !m_startingTest)
   {
      g_GameStatus.saveGameStatus();
   }

   m_startingTest = false;

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

// The clock of a scene. It runs at a fixed step (Scales::c_updateStep), so
// the scene is updated 60 times per second of real time: once per frame on a
// 60 Hz display, about every other frame at 120 Hz, twice per frame at 30 fps.
// (The physics used to step 1 / (frame rate measured at start-up) per frame,
// which ran slow whenever the game drew fewer frames than measured, e.g. on
// Android in Release, where the splash screen ran at the display's 120 Hz.)
// The clock's time starts half a step ahead, so the small jitter in frame
// times on a 60 Hz display doesn't give frames with 0 and then 2 updates
// (oxygine's Clock also snaps frame times within 1 ms of a whole step).
spClock MainActor::createSceneClock(void)
{
   spClock clock = new Clock;
   clock->setFixedStepF(Scales::c_updateStep);
   clock->update(getTimeMS() - (timeMS)(Scales::c_updateStep * 500.0f));
   return clock;
}

void MainActor::doUpdate(const UpdateState& us)
{
   if (m_splashArm)
   {
      m_splashArm = false;
      showSplashScreen();
      m_fpsEstArm = true;
      return;
   }

   if (m_fpsEstArm)
   {
      if (m_fpsEst == NULL)
      {
         m_fpsEst = new FpsEstimator(this);
         return;
      }
      else
      {
         if (m_fpsEst->getFps() < 0.0f)
         {
            // Do nothing
            return;
         }
         else
         {
            m_fps = m_fpsEst->getFps();

            logs::messageln("FPS estimated to: %f", m_fps);

            removeChild(m_fpsEst);

            m_fpsEstArm = false;
            m_initArm = true;
         }
      }
      return;
   }

   if (m_initArm)
   {
      initMainActor();
      m_initArm = false;
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



void MainActor::placeMessageDisplay(void)
{
   // On the left, above the turn buttons, in every scene (orbit too: its
   // display is between this column and the booster/fire buttons, see
   // HeadDownDisplay::orbitRect). As set up at start
   // (initialiseMessageDisplay in initMainActor).
   float bw = g_Layout.getButtonWidth();
   float height = g_Layout.getViewPortBounds().y;

   g_MessageDisplay->setColumn(
      g_Layout.getXFromLeft(0),
      g_Layout.getXFromLeft(2),
      0.0f,
      bw * 0.1f,
      height,
      g_Layout.getYFromBottom(1) - bw / 2.0f);
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
}

void MainActor::testStartClicked(int index)
{
   m_testStartIndex = index;
   m_postMenuAction = PostMenuActionType::testStart;
   g_HeadDownDisplay->goToMap();
}


void MainActor::restartedFromMenu(void)
{
   m_menuArm = true;
   m_clock->resume();
   m_isInPause = false;
 
   g_MessageDisplay->setFullHeight(false);

   if (m_postMenuAction == PostMenuActionType::continueCurrent)
   {
      // Release pause
      g_headUpDisplay->setTouchEnabled(true);
      g_headUpDisplay->showButtons(true);
      g_headUpDisplay->showStatusBars(m_nextScene.m_nextSceneType != SceneActor::SceneTypeEnum::orbit);

   }
   else if (m_postMenuAction == PostMenuActionType::startNew)
   {
      // Another game: the old one's messages go
      g_MessageDisplay->cleanAndClearMessageDisplay();

      // Initialise new game data and load first scene according to lua
      g_GameStatus.initializeGameStatusXmlDocument();
      g_GameStatus.gameStatusNewGameInit();
      g_LuaInterface.lua_forceCurrentScene();
      g_LuaInterface.lua_startInitialScene();
   }
   else if (m_postMenuAction == PostMenuActionType::continueLatest)
   {
      // The saved game, not the one played since: its messages go
      g_MessageDisplay->cleanAndClearMessageDisplay();

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
   else if (m_postMenuAction == PostMenuActionType::testStart)
   {
      // As the game gets there: the scripts set up the scene, mission,
      // state and the Leapfrog's entry (lua_prepareTestStart), the game
      // status follows, and the scene is armed
      TestStart start;
      std::string message;

      if (g_LuaInterface.lua_prepareTestStart(m_testStartIndex, start, message))
      {
         g_GameStatus.setSceneMissionState(
            start.m_scene, start.m_mission, start.m_state, start.m_sceneType,
            start.m_exitScene, start.m_exitType, start.m_exitParameter);
         m_startingTest = true;
         armScene(start.m_scene, start.m_sceneType);
      }
      else
      {
         logs::messageln("Test start refused: %s", message.c_str());

         if (g_HeadDownDisplay)
         {
            g_HeadDownDisplay->logMessage(message, true);
         }
      }
   }
}

void MainActor::menuStartTransitionComplete(void)
{
   spMenuActor ma = new MenuActor();
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

bool MainActor::isInPause(void)
{
   return m_isInPause || m_isInMessagePause;
}

bool MainActor::canPauseForMessages(void)
{
   return !m_isInPause && !m_isInMessagePause && m_clock && (m_sceneObject != NULL);
}

void MainActor::setMessagePause(bool pause)
{
   if (pause == m_isInMessagePause)
   {
      return;
   }

   m_isInMessagePause = pause;

   if (pause)
   {
      m_clock->pause();

      // The HUD won't see the fingers lift while it is disabled, so
      // release the flight controls now rather than having the booster
      // stuck on when the game continues
      m_sceneObject->m_turnLeftPressed = false;
      m_sceneObject->m_turnRightPressed = false;
      m_sceneObject->m_boosterPressed = false;
      m_sceneObject->m_firePressed = false;

      g_headUpDisplay->setTouchEnabled(false);
   }
   else
   {
      m_clock->resume();
      g_headUpDisplay->setTouchEnabled(true);
   }
}

void MainActor::goToPause(void)
{
   m_clock->pause();
   m_isInPause = true;

   // g_headUpDisplay->cleanAndRemove();
   g_headUpDisplay->showStatusBars(false);
   g_headUpDisplay->setTouchEnabled(false);
   g_headUpDisplay->showButtons(false);
   g_HeadDownDisplay->goToMenu();
   g_MessageDisplay->setFullHeight(true);

   m_menuArm = false;

}
