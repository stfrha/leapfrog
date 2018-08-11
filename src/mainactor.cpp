
#include <functional>
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include "physdispconvert.h"
#include "scales.h" 
#include "landingactorevents.h"
#include "deepspacesceneevents.h"

#include "mainactor.h"

using namespace oxygine;
using namespace std;

MainActor::MainActor()
{
	//load xml file with resources definition
	m_gameResources.loadXML("res.xml");

	setSize(getStage()->getSize());

   startScene(STE_LANDING);
//   startScene(STE_FREE_SPACE);
//   startScene(STE_ORBIT);
}

MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
}

void MainActor::startScene(SceneTypeEnum scene)
{
   // Clean up current scene
   spActor actor = getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
   	   actor->detach();
      }
      actor = next;
   }

   // Start selected scene
   if (scene == STE_LANDING)
   { 
      spClipRectActor window = new ClipRectActor();

      window->setSize(getStage()->getSize() - Vector2(100, 100));
      window->setPosition(50.0f, 50.0f);
      addChild(window);

      spLandingActor landingActor = new LandingActor(m_gameResources, string("landing_scene.xml"), string("landingState"));
      window->addChild(landingActor);
      landingActor->addEventListener(LandingActorTranstToDeepSpaceEvent::EVENT, CLOSURE(this, &MainActor::transitToDeepSpaceListner));

      m_sceneObject = static_cast<SceneActor*>(landingActor.get());

   }
   else if (scene == STE_FREE_SPACE)
   {
      spClipRectActor window = new ClipRectActor();

      window->setSize(getStage()->getSize() - Vector2(100, 100));
      window->setPosition(50.0f, 50.0f);
      addChild(window);

      spFreeSpaceActor freeSpaceActor = new FreeSpaceActor(m_gameResources, string("deep_space_scene.xml"), string("deepSpaceState"));
      window->addChild(freeSpaceActor);

      freeSpaceActor->addEventListener(DeepSpaceSceneTranstToOrbitEvent::EVENT, CLOSURE(this, &MainActor::transitToOrbitListner));

      m_sceneObject = static_cast<SceneActor*>(freeSpaceActor.get());

   }
   else if (scene == STE_ORBIT)
   {
      spClipRectActor window = new ClipRectActor();

      window->setSize(getStage()->getSize() - Vector2(100, 100));
      window->setPosition(50.0f, 50.0f);
      addChild(window);

      spOrbitScene reentryActor = new OrbitScene(m_gameResources, string("orbit_scene.xml"), string("deepSpaceState"));
      window->addChild(reentryActor);

      reentryActor->addEventListener(OrbitSceneLandingComplete::EVENT, CLOSURE(this, &MainActor::landingCompleteListner));

      m_sceneObject = (SceneActor*)reentryActor->m_space;
   }

   //m_debugDraw = new Box2DDraw;
   //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
   //m_debugDraw->attachTo(freeSpaceActor);
   //m_debugDraw->setWorld(Scales::c_physToStageScale, freeSpaceActor->GetWorld());
   //m_debugDraw->setPriority(1);

   createButtonOverlay();
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
   startScene(STE_FREE_SPACE);
}

void MainActor::transitToOrbitListner(Event *ev)
{
   startScene(STE_ORBIT);
}

void MainActor::landingCompleteListner(oxygine::Event *ev)
{
   startScene(STE_LANDING);
}

void MainActor::doUpdate(const UpdateState& us)
{
   const Uint8* data = SDL_GetKeyboardState(0);

   if (data[SDL_SCANCODE_F1])
   {
      startScene(STE_LANDING);
   }
   else if (data[SDL_SCANCODE_F2])
   {
      startScene(STE_FREE_SPACE);
   }
   else if (data[SDL_SCANCODE_F3])
   {
      startScene(STE_ORBIT);
   }
}


void MainActor::createButtonOverlay(void)
{
   // Each button is a tenth of the screen height
   float d = (getStage()->getSize()).y / 10.0f;

   // Define sprites
   spSprite turnLeftButton = new Sprite();
   turnLeftButton->setResAnim(m_gameResources.getResAnim("button"));
   turnLeftButton->setSize(d, d);
   turnLeftButton->setPosition(d / 2, (getStage()->getSize()).y - d - d - d + d / 2);
   turnLeftButton->setAnchor(0.5f, 0.5f);
   turnLeftButton->attachTo(this);
   turnLeftButton->setTouchEnabled(true);
   turnLeftButton->addTouchDownListener(CLOSURE(this, &MainActor::turnLeftButtonDownHandler));
   turnLeftButton->addTouchUpListener(CLOSURE(this, &MainActor::turnLeftButtonUpHandler));

   spSprite turnRightButton = new Sprite();
   turnRightButton->setResAnim(m_gameResources.getResAnim("button"));
   turnRightButton->setSize(d, d);
   turnRightButton->setPosition(d / 2 + d / 2, (getStage()->getSize()).y - d + d / 2);
   turnRightButton->setAnchor(0.5f, 0.5f);
   turnRightButton->attachTo(this);
   turnRightButton->setTouchEnabled(true);
   turnRightButton->addTouchDownListener(CLOSURE(this, &MainActor::turnRightButtonDownHandler));
   turnRightButton->addTouchUpListener(CLOSURE(this, &MainActor::turnRightButtonUpHandler));

   spSprite thursterButton = new Sprite();
   thursterButton->setResAnim(m_gameResources.getResAnim("button"));
   thursterButton->setSize(d, d);
   thursterButton->setPosition((getStage()->getSize()).x - d - d / 2 + d / 2, (getStage()->getSize()).y - d + d / 2);
   thursterButton->setAnchor(0.5f, 0.5f);
   thursterButton->attachTo(this);
   thursterButton->setTouchEnabled(true);
   thursterButton->addTouchDownListener(CLOSURE(this, &MainActor::boosterButtonDownHandler));
   thursterButton->addTouchUpListener(CLOSURE(this, &MainActor::boosterButtonUpHandler));

   spSprite fireButton = new Sprite();
   fireButton->setResAnim(m_gameResources.getResAnim("button"));
   fireButton->setSize(d, d);
   fireButton->setPosition((getStage()->getSize()).x - d + d / 2, (getStage()->getSize()).y - d - d - d + d / 2);
   fireButton->setAnchor(0.5f, 0.5f);
   fireButton->attachTo(this);
   fireButton->setTouchEnabled(true);
   fireButton->addTouchDownListener(CLOSURE(this, &MainActor::fireButtonDownHandler));
   fireButton->addTouchUpListener(CLOSURE(this, &MainActor::fireButtonUpHandler));

   spSprite zoomOutButton = new Sprite();
   zoomOutButton->setResAnim(m_gameResources.getResAnim("button"));
   zoomOutButton->setSize(d, d);
   zoomOutButton->setPosition((getStage()->getSize()).x - d - d - d / 2 + d / 2, d / 2);
   zoomOutButton->setAnchor(0.5f, 0.5f);
   zoomOutButton->attachTo(this);
   zoomOutButton->setTouchEnabled(true);
   zoomOutButton->addTouchDownListener(CLOSURE(this, &MainActor::zoomOutButtonDownHandler));
   zoomOutButton->addTouchUpListener(CLOSURE(this, &MainActor::zoomOutButtonUpHandler));

   spSprite zoomInButton = new Sprite();
   zoomInButton->setResAnim(m_gameResources.getResAnim("button"));
   zoomInButton->setSize(d, d);
   zoomInButton->setPosition((getStage()->getSize()).x - d + d / 2, d / 2);
   zoomInButton->setAnchor(0.5f, 0.5f);
   zoomInButton->attachTo(this);
   zoomInButton->setTouchEnabled(true);
   zoomInButton->addTouchDownListener(CLOSURE(this, &MainActor::zoomInButtonDownHandler));
   zoomInButton->addTouchUpListener(CLOSURE(this, &MainActor::zoomInButtonUpHandler));
}

void MainActor::turnLeftButtonDownHandler(Event* event)
{
   m_sceneObject->m_turnLeftPressed = true;
}

void MainActor::turnLeftButtonUpHandler(Event* event)
{
   m_sceneObject->m_turnLeftPressed = false;
}

void MainActor::turnRightButtonDownHandler(Event* event)
{
   m_sceneObject->m_turnRightPressed = true;
}

void MainActor::turnRightButtonUpHandler(Event* event)
{
   m_sceneObject->m_turnRightPressed = false;
}

void MainActor::boosterButtonDownHandler(Event* event)
{
   m_sceneObject->m_boosterPressed = true;
}

void MainActor::boosterButtonUpHandler(Event* event)
{
   m_sceneObject->m_boosterPressed = false;
}

void MainActor::fireButtonDownHandler(Event* event)
{
   m_sceneObject->m_firePressed = true;
}

void MainActor::fireButtonUpHandler(Event* event)
{
   m_sceneObject->m_firePressed = false;
}

void MainActor::zoomInButtonDownHandler(Event* event)
{
   m_sceneObject->m_zoomInPressed = true;
}

void MainActor::zoomInButtonUpHandler(Event* event)
{
   m_sceneObject->m_zoomInPressed = false;
}

void MainActor::zoomOutButtonDownHandler(Event* event)
{
   m_sceneObject->m_zoomOutPressed = true;
}

void MainActor::zoomOutButtonUpHandler(Event* event)
{
   m_sceneObject->m_zoomOutPressed = false;
}