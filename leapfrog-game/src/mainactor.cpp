
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
#include "messagedisplay.h"
#include "luainterface.h"

#include "objectresourcesevents.h"

using namespace oxygine;
using namespace std;

NextSceneDefinition::NextSceneDefinition() :
   m_nextSceneFile("landing_scene.xml"),
   m_armNextScene(true),
   m_nextSceneType(SceneActor::SceneTypeEnum::landing)
{
}

MainActor::MainActor() :
   m_world(NULL),
   m_turnRightTouchIndex(0),
   m_turnLeftTouchIndex(0),
   m_boosterTouchIndex(0),
   m_fireTouchIndex(0),
   m_zoomInTouchIndex(0),
   m_zoomOutTouchIndex(0),
   m_panTouchIndex(0),
   m_reloadTouchIndex(0),
   m_splashArm(true),
   m_reloadPressed(false),
   m_reloadArm(true)
{
   g_Layout.initLayout();

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
   m_gameResources.loadXML("res.xml");
   m_hudResources.loadXML("res_hud.xml");

   // Restore game status except leapfrog. That is 
   // managed on the Leapfrog object initiative
   g_GameStatus.restoreGameStatus();

   g_LuaInterface.initLuaInterface(this);

   g_HeadDownDisplay = new HeadDownDisplay();

   g_MessageDisplay = new MessageDisplay();

   g_MessageDisplay->initialiseMessageDisplay(
      &m_hudResources,
      getStage().get(),
      Vector2(0.0f, g_Layout.getButtonWidth()),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getYFromBottom(1) - g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize());

   setSize(g_Layout.getViewPortBounds());

   g_LuaInterface.lua_forceCurrentScene();
   g_LuaInterface.lua_startInitialScene();
   m_nextScene.m_armNextScene = true;

   //   addEventListener(TouchEvent::MOVE, CLOSURE(this, &MainActor::sceneMoveHandler));
   addEventListener(CompoundObjectCreatedEvent::EVENT, CLOSURE(this, &MainActor::dummyHandler));

}


MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
   m_hudResources.free();
}


void MainActor::armScene(std::string sceneName, int sceneType)
{
   m_nextScene.m_nextSceneFile = sceneName;
   m_nextScene.m_nextSceneType = static_cast<SceneActor::SceneTypeEnum>(sceneType);
   m_nextScene.m_armNextScene = true;
}

void MainActor::startScene(void)
{
   // Clear all triggers and events from the current 
   // actors before they are destroyed.
   g_LuaInterface.clearAllTriggersAndEvents();

   // Remove listners which includes scene transit events
   removeAllEventListeners();

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

   // And then re-add those needed by main actor
   addTouchDownListener(CLOSURE(this, &MainActor::sceneDownHandler));
   addTouchUpListener(CLOSURE(this, &MainActor::sceneUpHandler));
   addEventListener(TouchEvent::MOVE, (CLOSURE(this, &MainActor::panMoveHandler)));

   spClipRectActor window = new ClipRectActor();

   window->setSize(g_Layout.getViewPortBounds());
   window->setPosition(0.0f, 0.0f);
   addChild(window);

   if ((m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::landing) ||
      (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::deepSpace))
   {
      spSceneActor sceneObj = static_cast<SceneActor*>(CompoundObject::readDefinitionXmlFile(
            m_gameResources,
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
            m_gameResources,
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

   m_sceneObject->connectToForeignObjects();

   m_sceneObject->setManualPan(&m_manualPan);


   addEventListener(StatusResourceDepletedEvent::EVENT, CLOSURE(this, &MainActor::resourceDepletedHandler));

   g_HeadDownDisplay->initialiseMap(
      &m_hudResources,
      m_sceneObject,
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize());

   g_LuaInterface.setupMissionStateScene(m_sceneObject);

   createButtonOverlay();

   setManualPanButtonState();


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

void MainActor::setManualPanButtonState(void)
{
   if (m_manualPan.m_manPanEnable)
   {
      m_manPanEnableSprite->setAlpha(255);
      m_manPanSprite->setVisible(true);
      m_reloadSprite->setVisible(true);
      m_zoomInSprite->setVisible(true);
      m_zoomOutSprite->setVisible(true);
   }
   else
   {
      m_manPanEnableSprite->setAlpha(128);
      m_manPanSprite->setVisible(false);
      m_reloadSprite->setVisible(false);
      m_zoomInSprite->setVisible(false);
      m_zoomOutSprite->setVisible(false);
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
      const Uint8* data = SDL_GetKeyboardState(0);

     // if (data[SDL_SCANCODE_F1])
     // {
     //    g_LuaInterface.lua_forceCurrentScene("orbit_scene.xml");
     //    g_LuaInterface.determineNextScene("toLanding", "alphaCity", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
     //    m_nextScene.m_armNextScene = true;
     //}
     // else if (data[SDL_SCANCODE_F2])
     // {
     //    g_LuaInterface.lua_forceCurrentScene("landing_scene.xml");
     //    g_LuaInterface.determineNextScene("toDeepSpace", "", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
     //    m_nextScene.m_armNextScene = true;

     // }
     // else if (data[SDL_SCANCODE_F3])
     // {
     //    g_LuaInterface.lua_forceCurrentScene("deep_space_scene.xml");
     //    g_LuaInterface.determineNextScene("toOrbit", "", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
     //    m_nextScene.m_armNextScene = true;
     // }
   }

}


void MainActor::createButtonOverlay(void)
{
   // Caluclate button geometrics
   m_turnLeftButtonRect = RectF(0.0f, g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_turnRightButtonRect = RectF(g_Layout.getButtonWidth(), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_boosterButtonRect = RectF(g_Layout.getXFromRight(0), g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_fireButtonRect = RectF(g_Layout.getXFromRight(1), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());

   m_pauseButtonRect = RectF(g_Layout.getXFromRight(0), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanEnableButtonRect = RectF(g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInButtonRect = RectF(g_Layout.getXFromRight(2) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutButtonRect = RectF(g_Layout.getXFromRight(4), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_reloadButtonRect = RectF(g_Layout.getXFromRight(5) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_panButtonRect = RectF(g_Layout.getViewPortBounds().x / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getViewPortBounds().y / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());


   // Define sprites
   spSprite turnLeftButton = new Sprite();
   turnLeftButton->setResAnim(m_hudResources.getResAnim("turn_left_button"));
   turnLeftButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnLeftButton->setPosition(m_turnLeftButtonRect.getLeftTop());
   turnLeftButton->setAnchor(0.0f, 0.0f);
   turnLeftButton->setTouchEnabled(false);
   turnLeftButton->attachTo(this);

   spSprite turnRightButton = new Sprite();
   turnRightButton->setResAnim(m_hudResources.getResAnim("turn_right_button"));
   turnRightButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnRightButton->setPosition(m_turnRightButtonRect.getLeftTop());
   turnRightButton->setAnchor(0.0f, 0.0f);
   turnRightButton->setTouchEnabled(false);
   turnRightButton->attachTo(this);

   spSprite fireButton = new Sprite();
   fireButton->setResAnim(m_hudResources.getResAnim("fire_button"));
   fireButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   fireButton->setPosition(m_fireButtonRect.getLeftTop());
   fireButton->setAnchor(0.0f, 0.0f);
   fireButton->setTouchEnabled(false);
   fireButton->attachTo(this);

   spSprite thursterButton = new Sprite();
   thursterButton->setResAnim(m_hudResources.getResAnim("booster_button"));
   thursterButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   thursterButton->setPosition(m_boosterButtonRect.getLeftTop());
   thursterButton->setAnchor(0.0f, 0.0f);
   thursterButton->setTouchEnabled(false);
   thursterButton->attachTo(this);

   spSprite pauseButton = new Sprite();
   pauseButton->setResAnim(m_hudResources.getResAnim("pause_button"));
   pauseButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   pauseButton->setPosition(m_pauseButtonRect.getLeftTop());
   pauseButton->setAnchor(0.0f, 0.0f);
   pauseButton->setTouchEnabled(false);
   pauseButton->attachTo(this);

   m_zoomOutSprite = new Sprite();
   m_zoomOutSprite->setResAnim(m_hudResources.getResAnim("zoom_out_button"));
   m_zoomOutSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutSprite->setPosition(m_zoomOutButtonRect.getLeftTop());
   m_zoomOutSprite->setAnchor(0.0f, 0.0f);
   m_zoomOutSprite->setTouchEnabled(false);
   m_zoomOutSprite->attachTo(this);

   m_zoomInSprite = new Sprite();
   m_zoomInSprite->setResAnim(m_hudResources.getResAnim("zoom_in_button"));
   m_zoomInSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInSprite->setPosition(m_zoomInButtonRect.getLeftTop());
   m_zoomInSprite->setAnchor(0.0f, 0.0f);
   m_zoomInSprite->setTouchEnabled(false);
   m_zoomInSprite->attachTo(this);

   m_manPanEnableSprite = new Sprite();
   m_manPanEnableSprite->setResAnim(m_hudResources.getResAnim("pan_button"));
   m_manPanEnableSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanEnableSprite->setPosition(m_manPanEnableButtonRect.getLeftTop());
   m_manPanEnableSprite->setAnchor(0.0f, 0.0f);
   m_manPanEnableSprite->setTouchEnabled(false);
   m_manPanEnableSprite->attachTo(this);

   m_manPanSprite = new Sprite();
   m_manPanSprite->setResAnim(m_hudResources.getResAnim("pan_button"));
   m_manPanSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanSprite->setPosition(m_panButtonRect.getLeftTop());
   m_manPanSprite->setAnchor(0.0f, 0.0f);
   m_manPanSprite->setTouchEnabled(false);
   m_manPanSprite->attachTo(this);

   m_reloadSprite = new Sprite();
   m_reloadSprite->setResAnim(m_hudResources.getResAnim("reload_button"));
   m_reloadSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_reloadSprite->setPosition(m_reloadButtonRect.getLeftTop());
   m_reloadSprite->setAnchor(0.0f, 0.0f);
   m_reloadSprite->setTouchEnabled(false);
   m_reloadSprite->attachTo(this);
  
   //spBox9Sprite hej = new Box9Sprite();
   //hej->setResAnim(m_gameResources.getResAnim("box9a"));
   //hej->setVerticalMode(Box9Sprite::TILING);
   //hej->setHorizontalMode(Box9Sprite::TILING);
   //hej->setSize(512.0f, 256.0f);
   //hej->setPosition(700.0f, 250.0f);
   //hej->setAnchor(0.0f, 0.0f);
   //hej->setGuides(40, 160, 40, 160);
   //hej->setTouchEnabled(false);
   //hej->attachTo(this);

   //spBox9Sprite hej2 = new Box9Sprite();
   //hej2->setResAnim(m_gameResources.getResAnim("box9a"));
   //hej2->setVerticalMode(Box9Sprite::STRETCHING);
   //hej2->setHorizontalMode(Box9Sprite::STRETCHING);
   //hej2->setSize(512, 256.0f);
   //hej2->setPosition(700.0f, 530.0f);
   //hej2->setAnchor(0.0f, 0.0f);
   //hej->setGuides(40, 160, 40, 160);
   //hej2->setTouchEnabled(false);
   //hej2->attachTo(this);

}


void MainActor::sceneDownHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;
   Vector2 v = te->localPosition;

   // logs::messageln("Down with index: %d", te->index);

   Point p = Point((int)v.x, (int)v.y);

   if (m_turnLeftButtonRect.pointIn(p))
   {
      m_sceneObject->m_turnLeftPressed = true;
      m_turnLeftTouchIndex = te->index;
   }
   else if (m_turnRightButtonRect.pointIn(p))
   {
      m_sceneObject->m_turnRightPressed = true;
      m_turnRightTouchIndex = te->index;
   }
   else if (m_boosterButtonRect.pointIn(p))
   {
      m_sceneObject->m_boosterPressed = true;
      m_boosterTouchIndex = te->index;
   }
   else if (m_fireButtonRect.pointIn(p))
   {
      m_sceneObject->m_firePressed = true;
      m_fireTouchIndex = te->index;
   }
   else if (m_pauseButtonRect.pointIn(p))
   {
      m_sceneObject->m_pausePressed = true;
      m_pauseTouchIndex = te->index;
   }
   else if (m_zoomInButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneObject->m_zoomInPressed = true;
         m_zoomInTouchIndex = te->index;
      }
   }
   else if (m_zoomOutButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneObject->m_zoomOutPressed = true;
         m_zoomOutTouchIndex = te->index;
      }
   }
   else if (m_manPanEnableButtonRect.pointIn(p))
   {
      m_sceneObject->m_manPanEnablePressed = true;
      m_manPanEnableTouchIndex = te->index;
   }
   else if (m_panButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneObject->m_panButtonPressed = true;
         m_panTouchIndex = te->index;
         m_panStartPos = v;
      }
   }
   else if (m_reloadButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_reloadPressed = true;
         m_reloadTouchIndex = te->index;
      }
   }
}


void MainActor::sceneUpHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;

   //logs::messageln("Up with index: %d", te->index);

   // Vector2 v = te->localPosition;
   // Point p = Point((int)v.x, (int)v.y);

   if (te->index == m_turnLeftTouchIndex)
   {
      m_sceneObject->m_turnLeftPressed = false;
      m_turnLeftTouchIndex = 0;
   }
   else if (te->index == m_turnRightTouchIndex)
   {
      m_sceneObject->m_turnRightPressed = false;
      m_turnRightTouchIndex = 0;
   }
   else if (te->index == m_boosterTouchIndex)
   {
      m_sceneObject->m_boosterPressed = false;
      m_boosterTouchIndex = 0;
   }
   else if (te->index == m_fireTouchIndex)
   {
      m_sceneObject->m_firePressed = false;
      m_fireTouchIndex = 0;
   }
   else if (te->index == m_pauseTouchIndex)
   {
      m_sceneObject->m_pausePressed = false;
      m_pauseTouchIndex = 0;
   }
   else if (te->index == m_zoomInTouchIndex)
   {
      m_sceneObject->m_zoomInPressed = false;
      m_zoomInTouchIndex = 0;
   }
   else if (te->index == m_zoomOutTouchIndex)
   {
      m_sceneObject->m_zoomOutPressed = false;
      m_zoomOutTouchIndex = 0;
   }
   else if (te->index == m_manPanEnableTouchIndex)
   {
      m_sceneObject->m_manPanEnablePressed = false;
      m_manPanEnableTouchIndex = 0;

      setManualPanButtonState();
   }
   else if (te->index == m_panTouchIndex)
   {
      m_sceneObject->m_panButtonPressed = false;
      m_sceneObject->m_panVector = Vector2(0.0f, 0.0f);
      m_panTouchIndex = 0;
   }
   else if (te->index == m_reloadTouchIndex)
   {
      m_reloadPressed = false;
      m_reloadTouchIndex = 0;
   }
}

void MainActor::panMoveHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;

   if (te->index == m_panTouchIndex)
   {
      m_sceneObject->m_panVector = (te->localPosition - m_panStartPos) / 8.0f;
   }
}

void MainActor::sceneMoveHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;
   Vector2 v = te->localPosition;

   //logs::messageln("Move with index: %d", te->index);

}


void MainActor::dummyHandler(Event* event)
{
   logs::messageln("Got dummy event");
}

void MainActor::recursiveRemoveChildren(spActor& parent)
{
   spActor child = parent->getFirstChild();

   while (child != NULL)
   {
      spActor copy = child;

      recursiveRemoveChildren(child);
      child.get()->detach();

      child = copy->getNextSibling();
   }
}

void MainActor::registerLeapfrog(LeapFrog* leapfrog)
{
   if (leapfrog == NULL)
   {
      return;
   }

   float initShots = 100.0f;
   float initFuel = 100.0f;
   float initShield = 100.0f;
   float initDamage = 100.0f;
   float initCredits = 0.0f;

   if (leapfrog != NULL)
   {
      initShots = (float)g_GameStatus.getResources()->getShots();
      initFuel = g_GameStatus.getResources()->getFuel();
      initShield = g_GameStatus.getResources()->getShield();
      initDamage = g_GameStatus.getResources()->getDamage();
      initCredits = (float)g_GameStatus.getResources()->getCredits();
   }

   spStatusBar shotsBar = new StatusBar(
      m_hudResources,
      leapfrog,
      m_sceneObject,
      5,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1)),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initShots,
      "Ammo:",
      ObjectResourcesTypeEnum::shots);

   spStatusBar fuelBar = new StatusBar(
      m_hudResources,
      leapfrog,
      m_sceneObject,
      7,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initFuel,
      "Fuel:",
      ObjectResourcesTypeEnum::fuel);

   spStatusBar shieldBar = new StatusBar(
      m_hudResources,
      leapfrog,
      m_sceneObject,
      6,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initShield,
      "Shield:",
      ObjectResourcesTypeEnum::shield);

   spStatusBar damageBar = new StatusBar(
      m_hudResources,
      leapfrog,
      m_sceneObject,
      9,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 3.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initDamage,
      "Damage:",
      ObjectResourcesTypeEnum::damage);

   spStatusLiteral creditBar = new StatusLiteral(
      m_hudResources,
      leapfrog,
      m_sceneObject,
      8,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 4.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      (int)initCredits,
      "Credits:",
      ObjectResourcesTypeEnum::credits);

   if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::landing)
   {
      shieldBar->setAlpha(128);
   }

}