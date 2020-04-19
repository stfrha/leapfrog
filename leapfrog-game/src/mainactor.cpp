
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

#include "gamestatusevents.h"

using namespace oxygine;
using namespace std;

NextSceneDefinition::NextSceneDefinition() :
   m_nextSceneFile("landing_scene.xml"),
   m_armNextScene(true),
   m_nextSceneType(SceneActor::SceneTypeEnum::landing),
   m_nextSceneState("default")
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
   m_reloadPressed(false),
   m_reloadArm(true)
{
   g_Layout.initLayout();
   
	//load xml file with resources definition
	m_gameResources.loadXML("res.xml");

   g_LuaInterface.initLuaInterface();

   g_HeadDownDisplay = new HeadDownDisplay();

   g_MessageDisplay = new MessageDisplay();

   g_MessageDisplay->initialiseMessageDisplay(
      &m_gameResources,
      getStage().get(),
      Vector2(0.0f, g_Layout.getButtonWidth()),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getYFromBottom(1) - g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize());


   // Here the game status should probably be read from file

   //m_gameStatus->initGameStatus(this);

	setSize(g_Layout.getViewPortBounds());
   
   g_LuaInterface.determineNextScene(
      "New Game", "", 
      m_nextScene.m_nextSceneFile, 
      m_nextScene.m_nextSceneState, 
      m_nextScene.m_nextSceneType);
   m_nextScene.m_armNextScene = true;


   //g_LuaInterface.determineNextScene("toDeepSpace", "", m_nextSceneFile, m_nextSceneState, m_nextSceneType);
   //g_LuaInterface.determineNextScene("toOrbit", "", m_nextSceneFile, m_nextSceneState, m_nextSceneType);
   //g_LuaInterface.determineNextScene("toLanding", "alphaCity", m_nextSceneFile, m_nextSceneState, m_nextSceneType);

   //m_nextSceneFile = "landing_scene.xml";
   //m_armNextScene = true;
   //m_nextSceneType = SceneActor::SceneTypeEnum::landing;

   //m_nextSceneFile = "deep_space_scene.xml";
   //m_armNextScene = true;
   //m_nextSceneType = deepSpace;

   //m_nextSceneFile = "orbit_scene.xml";
   //m_armNextScene = true;
   //m_nextSceneType = orbit;

   //   addEventListener(TouchEvent::MOVE, CLOSURE(this, &MainActor::sceneMoveHandler));
   addEventListener(CompoundObjectCreatedEvent::EVENT, CLOSURE(this, &MainActor::dummyHandler));

}

MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
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
         sceneObj->addEventListener(LandingActorTransitToDeepSpaceEvent::EVENT, CLOSURE(this, &MainActor::transitToDeepSpaceListner));
      }
      else
      {
         m_sceneObject = sceneObj.get();
         sceneObj->addEventListener(DeepSpaceSceneTransitToOrbitEvent::EVENT, CLOSURE(this, &MainActor::transitToOrbitListner));
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

      co.get()->addEventListener(OrbitSceneLandingComplete::EVENT, CLOSURE(this, &MainActor::landingCompleteListner));
   }

   m_sceneObject->connectToForeignObjects();

   m_sceneObject->setManualPan(&m_manualPan);

   // Init game status in leapfrog object
   CompoundObject* leapfrog = m_sceneObject->getObject("leapfrog1");

   if (leapfrog != NULL)
   {
      leapfrog->initGameStatus(this);
   }
   else
   {
      // We should terminate here since there is unclear what to do if leapfrog is not in da house
   }


   spStatusBar shotsBar = new StatusBar(
      m_gameResources,
      leapfrog,
      m_sceneObject,
      5,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1)),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      (float)leapfrog->m_gameStatus->getShots(),
      "Ammo:",
      GameStatusTypeEnum::shots);

   spStatusBar fuelBar = new StatusBar(
      m_gameResources,
      leapfrog,
      m_sceneObject,
      7,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      (float)leapfrog->m_gameStatus->getFuel(),
      "Fuel:",
      GameStatusTypeEnum::fuel);

   spStatusBar shieldBar = new StatusBar(
      m_gameResources,
      leapfrog,
      m_sceneObject,
      6,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      (float)leapfrog->m_gameStatus->getShield(),
      "Shield:",
      GameStatusTypeEnum::shield);

   spStatusBar damageBar = new StatusBar(
      m_gameResources,
      leapfrog,
      m_sceneObject,
      9,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 3.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      (float)leapfrog->m_gameStatus->getDamage(),
      "Damage:",
      GameStatusTypeEnum::damage);

   spStatusLiteral creditBar = new StatusLiteral(
      m_gameResources,
      leapfrog,
      m_sceneObject,
      8,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 4.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      (float)leapfrog->m_gameStatus->getCredits(),
      "Credits:",
      GameStatusTypeEnum::credits);

   addEventListener(StatusResourceDepletedEvent::EVENT, CLOSURE(this, &MainActor::resourceDepletedHandler));

   g_HeadDownDisplay->initialiseMap(
      &m_gameResources,
      m_sceneObject,
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize());

   // Now that the map is initialised, we tell child objects to register on the 
   // map
   m_sceneObject->registerObjectsToMap();

   if (m_nextScene.m_nextSceneType == SceneActor::SceneTypeEnum::landing)
   {
      shieldBar->setAlpha(128);
   }

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

void MainActor::sendEvent(int eventId)
{


}


void MainActor::transitToDeepSpaceListner(Event *ev)
{
   fadeFromLanding();
}

void MainActor::fadeFromLanding(void)
{
   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(m_sceneObject->getPosition());
   fader->setSize(
      g_Layout.getViewPortBounds().x * m_sceneObject->getScale().x, 
      g_Layout.getViewPortBounds().y * m_sceneObject->getScale().y);
   fader->setAlpha(0);
   fader->setPriority(216);
   fader->attachTo(m_sceneObject);
   spTween tween = fader->addTween(Actor::TweenAlpha(255), 500);

   tween->setDoneCallback(CLOSURE(this, &MainActor::goToDeepSpaceFader));
}

void MainActor::goToDeepSpaceFader(Event *ev)
{
   g_LuaInterface.determineNextScene("toDeepSpace", "", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
   m_nextScene.m_armNextScene = true;

   //m_armNextScene = true;
   //m_nextSceneFile = "deep_space_scene.xml";
   //m_nextSceneType = SceneActor::SceneTypeEnum::deepSpace;

   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(m_sceneObject->getPosition().x - 200.0f, m_sceneObject->getPosition().y + 200.0f);
   fader->setSize(
      g_Layout.getViewPortBounds().x * m_sceneObject->getScale().x, 
      g_Layout.getViewPortBounds().y * m_sceneObject->getScale().y);
   fader->setAlpha(255);
   fader->setPriority(216);
   fader->attachTo(m_sceneObject);
   spTween tween = fader->addTween(Actor::TweenAlpha(0), 1000);
}

void MainActor::transitToOrbitListner(Event *ev)
{
   g_LuaInterface.determineNextScene("toOrbit", "", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
   m_nextScene.m_armNextScene = true;

   //m_armNextScene = true;
   //m_nextSceneFile = "orbit_scene.xml";
   //m_nextSceneType = SceneActor::SceneTypeEnum::orbit;
}

void MainActor::landingCompleteListner(oxygine::Event *ev)
{
   g_LuaInterface.determineNextScene("toLanding", "alphaCity", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
   m_nextScene.m_armNextScene = true;

   //m_armNextScene = true;
   //m_nextSceneFile = "landing_scene.xml";
   //m_nextSceneType = SceneActor::SceneTypeEnum::landing;
}

void MainActor::resourceDepletedHandler(oxygine::Event *ev)
{
   StatusResourceDepletedEvent* e = (StatusResourceDepletedEvent*)ev;


   switch (e->m_type)
   {
   case fuel:
      // Stop current burning 
      m_sceneObject->m_leapfrog->m_boosterFlame->stopEmitter();
      m_sceneObject->m_leapfrog->m_leftSteerFlame->stopEmitter();
      m_sceneObject->m_leapfrog->m_rightSteerFlame->stopEmitter();

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
   m_nextScene.m_nextSceneState = "landingState";
   m_nextScene.m_armNextScene = true;
   m_nextScene.m_nextSceneFile = "updated_scene.xml";

   ox::HttpRequestTask::release();


}

void MainActor::doUpdate(const UpdateState& us)
{
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

      if (data[SDL_SCANCODE_F1])
      {
         g_LuaInterface.forceCurrentScene("orbit_scene.xml");
         g_LuaInterface.determineNextScene("toLanding", "alphaCity", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
         m_nextScene.m_armNextScene = true;
     }
      else if (data[SDL_SCANCODE_F2])
      {
         g_LuaInterface.forceCurrentScene("landing_scene.xml");
         fadeFromLanding();

      }
      else if (data[SDL_SCANCODE_F3])
      {
         g_LuaInterface.forceCurrentScene("deep_space_scene.xml");
         g_LuaInterface.determineNextScene("toOrbit", "", m_nextScene.m_nextSceneFile, m_nextScene.m_nextSceneState, m_nextScene.m_nextSceneType);
         m_nextScene.m_armNextScene = true;
      }
   }

}


void MainActor::createButtonOverlay(void)
{
   //float division = 8.0f;
   //float buttonWidth = (getStage()->getSize()).y / division;
   //float halfButtonWidth = buttonWidth / 2;
   //float bottomRow = (getStage()->getSize()).y - buttonWidth;
   //float aboveBottomRow = bottomRow - buttonWidth;
   //float colLast = (getStage()->getSize()).x - buttonWidth;
   //float colSecondLast = colLast - buttonWidth;

   // Caluclate button geometrics
   m_turnLeftButtonRect = Rect(0.0f, g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_turnRightButtonRect = Rect(g_Layout.getButtonWidth(), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_boosterButtonRect = Rect(g_Layout.getXFromRight(0), g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_fireButtonRect = Rect(g_Layout.getXFromRight(1), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());

   m_manPanEnableButtonRect = Rect(g_Layout.getXFromRight(0), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInButtonRect = Rect(g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutButtonRect = Rect(g_Layout.getXFromRight(3), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_reloadButtonRect = Rect(g_Layout.getXFromRight(4) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_panButtonRect = Rect(g_Layout.getViewPortBounds().x / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getViewPortBounds().y / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());


   // Define sprites
   spSprite turnLeftButton = new Sprite();
   turnLeftButton->setResAnim(m_gameResources.getResAnim("turn_left_button"));
   turnLeftButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnLeftButton->setPosition(m_turnLeftButtonRect.getLeftTop());
   turnLeftButton->setAnchor(0.0f, 0.0f);
   turnLeftButton->setTouchEnabled(false);
   turnLeftButton->attachTo(this);

   spSprite turnRightButton = new Sprite();
   turnRightButton->setResAnim(m_gameResources.getResAnim("turn_right_button"));
   turnRightButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnRightButton->setPosition(m_turnRightButtonRect.getLeftTop());
   turnRightButton->setAnchor(0.0f, 0.0f);
   turnRightButton->setTouchEnabled(false);
   turnRightButton->attachTo(this);

   spSprite fireButton = new Sprite();
   fireButton->setResAnim(m_gameResources.getResAnim("fire_button"));
   fireButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   fireButton->setPosition(m_fireButtonRect.getLeftTop());
   fireButton->setAnchor(0.0f, 0.0f);
   fireButton->setTouchEnabled(false);
   fireButton->attachTo(this);

   spSprite thursterButton = new Sprite();
   thursterButton->setResAnim(m_gameResources.getResAnim("booster_button"));
   thursterButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   thursterButton->setPosition(m_boosterButtonRect.getLeftTop());
   thursterButton->setAnchor(0.0f, 0.0f);
   thursterButton->setTouchEnabled(false);
   thursterButton->attachTo(this);

   m_zoomOutSprite = new Sprite();
   m_zoomOutSprite->setResAnim(m_gameResources.getResAnim("zoom_out_button"));
   m_zoomOutSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutSprite->setPosition(m_zoomOutButtonRect.getLeftTop());
   m_zoomOutSprite->setAnchor(0.0f, 0.0f);
   m_zoomOutSprite->setTouchEnabled(false);
   m_zoomOutSprite->attachTo(this);

   m_zoomInSprite = new Sprite();
   m_zoomInSprite->setResAnim(m_gameResources.getResAnim("zoom_in_button"));
   m_zoomInSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInSprite->setPosition(m_zoomInButtonRect.getLeftTop());
   m_zoomInSprite->setAnchor(0.0f, 0.0f);
   m_zoomInSprite->setTouchEnabled(false);
   m_zoomInSprite->attachTo(this);

   m_manPanEnableSprite = new Sprite();
   m_manPanEnableSprite->setResAnim(m_gameResources.getResAnim("pan_button"));
   m_manPanEnableSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanEnableSprite->setPosition(m_manPanEnableButtonRect.getLeftTop());
   m_manPanEnableSprite->setAnchor(0.0f, 0.0f);
   m_manPanEnableSprite->setTouchEnabled(false);
   m_manPanEnableSprite->attachTo(this);

   m_manPanSprite = new Sprite();
   m_manPanSprite->setResAnim(m_gameResources.getResAnim("pan_button"));
   m_manPanSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanSprite->setPosition(m_panButtonRect.getLeftTop());
   m_manPanSprite->setAnchor(0.0f, 0.0f);
   m_manPanSprite->setTouchEnabled(false);
   m_manPanSprite->attachTo(this);

   m_reloadSprite = new Sprite();
   m_reloadSprite->setResAnim(m_gameResources.getResAnim("reload_button"));
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

   logs::messageln("Up with index: %d", te->index);

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

   logs::messageln("Move with index: %d", te->index);

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
