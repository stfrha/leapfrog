
#include <functional>
#include "mainactor.h"

#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include "physdispconvert.h"
#include "scales.h" 
#include "landingactorevents.h"
#include "deepspacesceneevents.h"
#include "layout.h"
#include "gamestatus.h"
#include "statusbar.h"
#include "orbitscene.h"
#include "orbitspacescene.h"
#include "headdowndisplay.h"
#include "messagedisplay.h"

#include "gamestatusevents.h"

using namespace oxygine;
using namespace std;

MainActor::MainActor() :
   m_world(NULL),
   m_nextSceneFile("landing_scene.xml"),
   m_armNextScene(true),
   m_nextSceneType(STE_LANDING),
   m_turnRightTouchIndex(0),
   m_turnLeftTouchIndex(0),
   m_boosterTouchIndex(0),
   m_fireTouchIndex(0),
   m_zoomInTouchIndex(0),
   m_zoomOutTouchIndex(0)
{
   g_Layout.initLayout();
   
	//load xml file with resources definition
	m_gameResources.loadXML("res.xml");

   m_gameStatus = new GameStatus();

   g_HeadDownDisplay = new HeadDownDisplay();

   g_MessageDisplay = new MessageDisplay();

   g_MessageDisplay->initialiseMessageDisplay(
      &m_gameResources,
      getStage().get(),
      Vector2(0.0f, g_Layout.getButtonWidth()),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getYFromBottom(1) - g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize());


   // Here the game status should probably be read from file

   m_gameStatus->initGameStatus(this);

	setSize(getStage()->getSize());

   m_nextSceneFile = "landing_scene.xml";
   m_armNextScene = true;
   m_nextSceneType = STE_LANDING;

   //m_nextSceneFile = "deep_space_scene.xml";
   //m_armNextScene = true;
   //m_nextSceneType = STE_FREE_SPACE;

   //m_nextSceneFile = "orbit_scene.xml";
   //m_armNextScene = true;
   //m_nextSceneType = STE_ORBIT;

   //   addEventListener(TouchEvent::MOVE, CLOSURE(this, &MainActor::sceneMoveHandler));

}

MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
}

void MainActor::startScene(void)
{
   // Remove listners for statusbars
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



   spClipRectActor window = new ClipRectActor();

   window->setSize(getStage()->getSize());
   window->setPosition(0.0f, 0.0f);
   addChild(window);

   //string fileName;
   string initialState = "default";

   if ((m_nextSceneType == STE_LANDING) || (m_nextSceneType == STE_FREE_SPACE))
   {
      spSceneActor sceneObj = static_cast<SceneActor*>(CompoundObject::readDefinitionXmlFile(
         m_gameResources,
         NULL,
         NULL,
         m_world,
         Vector2(0.0f, 0.0f),
         m_nextSceneFile,
         initialState));

      if (sceneObj == NULL)
      {
         return;
      }

      window->addChild(sceneObj);

      if (m_nextSceneType == STE_LANDING)
      {
         m_sceneObject = sceneObj.get();
         sceneObj->addEventListener(LandingActorTranstToDeepSpaceEvent::EVENT, CLOSURE(this, &MainActor::transitToDeepSpaceListner));
      }
      else
      {
         m_sceneObject = sceneObj.get();
         sceneObj->addEventListener(DeepSpaceSceneTranstToOrbitEvent::EVENT, CLOSURE(this, &MainActor::transitToOrbitListner));
      }
   }
   else if (m_nextSceneType == STE_ORBIT)
   {
      spCompoundObject co = CompoundObject::readDefinitionXmlFile(
         m_gameResources, 
         NULL, 
         NULL, 
         m_world, 
         Vector2(0.0f, 0.0f), 
         m_nextSceneFile, 
         initialState);

      window->addChild(co);

      OrbitScene* os = static_cast<OrbitScene*>(co.get());
      m_sceneObject = os->m_space;

      co.get()->addEventListener(OrbitSceneLandingComplete::EVENT, CLOSURE(this, &MainActor::landingCompleteListner));
   }

   m_sceneObject->connectToForeignObjects();

   // Init game status in leapfrog object
   CompoundObject* leapfrog = m_sceneObject->getObject("leapfrog1");

   if (leapfrog != NULL)
   {
      leapfrog->initGameStatus(m_gameStatus);
   }


   spStatusBar shotsBar = new StatusBar(
      m_gameResources,
      this,
      m_sceneObject,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(2)),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      m_gameStatus->getShots(),
      "Ammo:",
      GameStatusTypeEnum::shots);

   spStatusBar fuelBar = new StatusBar(
      m_gameResources,
      this,
      m_sceneObject,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(2) + g_Layout.getButtonWidth() / 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      m_gameStatus->getFuel(),
      "Fuel:",
      GameStatusTypeEnum::fuel);

   spStatusBar shieldBar = new StatusBar(
      m_gameResources,
      this,
      m_sceneObject,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(2) + g_Layout.getButtonWidth() / 2.0f * 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      m_gameStatus->getShield(),
      "Shield:",
      GameStatusTypeEnum::shield);

   spStatusBar damageBar = new StatusBar(
      m_gameResources,
      this,
      m_sceneObject,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(2) + g_Layout.getButtonWidth() / 2.0f * 3.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      g_Layout.getDefaultFontSize(),
      100.0f,
      m_gameStatus->getDamage(),
      "Damage:",
      GameStatusTypeEnum::damage);

   addEventListener(StatusResourceDepletedEvent::EVENT, CLOSURE(this, &MainActor::resourceDepletedHandler));

   g_HeadDownDisplay->initialiseMap(
      &m_gameResources,
      m_sceneObject,
      Vector2(0.0f, 0.0f),
      m_sceneObject->getSize());

   g_MessageDisplay->initMessage(
      true,
      "Hello! Hmm... are you... The driver of the leapfrog? I have meaning to talk to you for quite some time. You see, we are having a slight problem with Space Invaders in these here parts.",
      "Dr Lando");

   g_MessageDisplay->initMessage(
      false,
      "Look, I have not got the time.",
      "Me");

   g_MessageDisplay->initMessage(
      true,
      "We can pay you. Quite..",
      "Dr Lando");

   g_MessageDisplay->initMessage(
      false,
      "How much?",
      "Me");

   g_MessageDisplay->initMessage(
      true,
      "20 000 credits. More than enough.",
      "Dr Lando");

   g_MessageDisplay->initMessage(
      false,
      "Local currency?",
      "Me");

   // Now that the map is initialised, we tell child objects to register on the 
   // map
   m_sceneObject->registerObjectsToMap();

   if (m_nextSceneType == STE_LANDING)
   {
      shieldBar->setAlpha(128);
   }


   createButtonOverlay();

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
   fader->setSize(getStage()->getSize().x * m_sceneObject->getScale().x, getStage()->getSize().y * m_sceneObject->getScale().y);
   fader->setAlpha(0.0f);
   fader->setPriority(216);
   fader->attachTo(m_sceneObject);
   spTween tween = fader->addTween(Actor::TweenAlpha(255.0f), 500);

   tween->setDoneCallback(CLOSURE(this, &MainActor::goToDeepSpaceFader));
}

void MainActor::goToDeepSpaceFader(Event *ev)
{
   m_armNextScene = true;
   m_nextSceneFile = "deep_space_scene.xml";
   m_nextSceneType = STE_FREE_SPACE;

   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(m_sceneObject->getPosition().x - 200.0f, m_sceneObject->getPosition().y + 200.0f);
   fader->setSize(getStage()->getSize().x * m_sceneObject->getScale().x, getStage()->getSize().y * m_sceneObject->getScale().y);
   fader->setAlpha(255.0f);
   fader->setPriority(216);
   fader->attachTo(m_sceneObject);
   spTween tween = fader->addTween(Actor::TweenAlpha(0.0f), 1000);
}

void MainActor::transitToOrbitListner(Event *ev)
{
   m_armNextScene = true;
   m_nextSceneFile = "orbit_scene.xml";
   m_nextSceneType = STE_ORBIT;
}

void MainActor::landingCompleteListner(oxygine::Event *ev)
{
   m_armNextScene = true;
   m_nextSceneFile = "landing_scene.xml";
   m_nextSceneType = STE_LANDING;
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

void MainActor::doUpdate(const UpdateState& us)
{
   if (m_armNextScene)
   {
      m_armNextScene = false;
      startScene();
   }
   else
   {
      const Uint8* data = SDL_GetKeyboardState(0);

      if (data[SDL_SCANCODE_F1])
      {
         m_armNextScene = true;
         m_nextSceneFile = "landing_scene.xml";
         m_nextSceneType = STE_LANDING;
     }
      else if (data[SDL_SCANCODE_F2])
      {
         //      startScene(STE_FREE_SPACE);
         fadeFromLanding();

      }
      else if (data[SDL_SCANCODE_F3])
      {
         m_armNextScene = true;
         m_nextSceneFile = "orbit_scene.xml";
         m_nextSceneType = STE_ORBIT;
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
   m_turnLeftButtonRect = Rect(0, g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_turnRightButtonRect = Rect(g_Layout.getButtonWidth(), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_boosterButtonRect = Rect(g_Layout.getXFromRight(0), g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_fireButtonRect = Rect(g_Layout.getXFromRight(1), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInButtonRect = Rect(g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 2.0f, 0, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutButtonRect = Rect(g_Layout.getXFromRight(0), 0, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());

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

   spSprite zoomOutButton = new Sprite();
   zoomOutButton->setResAnim(m_gameResources.getResAnim("zoom_out_button"));
   zoomOutButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   zoomOutButton->setPosition(m_zoomOutButtonRect.getLeftTop());
   zoomOutButton->setAnchor(0.0f, 0.0f);
   zoomOutButton->setTouchEnabled(false);
   zoomOutButton->attachTo(this);

   spSprite zoomInButton = new Sprite();
   zoomInButton->setResAnim(m_gameResources.getResAnim("zoom_in_button"));
   zoomInButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   zoomInButton->setPosition(m_zoomInButtonRect.getLeftTop());
   zoomInButton->setAnchor(0.0f, 0.0f);
   zoomInButton->setTouchEnabled(false);
   zoomInButton->attachTo(this);
}

//void MainActor::turnLeftButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_turnLeftPressed = true;
//}
//
//void MainActor::turnLeftButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_turnLeftPressed = false;
//}
//
//void MainActor::turnRightButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_turnRightPressed = true;
//}
//
//void MainActor::turnRightButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_turnRightPressed = false;
//}
//
//void MainActor::boosterButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_boosterPressed = true;
//}
//
//void MainActor::boosterButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_boosterPressed = false;
//}
//
//void MainActor::fireButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_firePressed = true;
//}
//
//void MainActor::fireButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_firePressed = false;
//}
//
//void MainActor::zoomInButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_zoomInPressed = true;
//}
//
//void MainActor::zoomInButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_zoomInPressed = false;
//}
//
//void MainActor::zoomOutButtonDownHandler(Event* event)
//{
//   m_sceneObject->m_zoomOutPressed = true;
//}
//
//void MainActor::zoomOutButtonUpHandler(Event* event)
//{
//   m_sceneObject->m_zoomOutPressed = false;
//}

void MainActor::sceneDownHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;
   Vector2 v = te->localPosition;

   logs::messageln("Down with index: %d", te->index);

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
      m_sceneObject->m_zoomInPressed = true;
      m_zoomInTouchIndex = te->index;
   }
   else if (m_zoomOutButtonRect.pointIn(p))
   {
      m_sceneObject->m_zoomOutPressed = true;
      m_zoomOutTouchIndex = te->index;
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
