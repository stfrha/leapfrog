
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
	m_gameResources.loadXML("..\\data\\res.xml");

	setSize(getStage()->getSize());

//   startScene(STE_LANDING);
//   startScene(STE_FREE_SPACE);
   startScene(STE_ORBIT);
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
      if (actor != NULL)
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

      m_sceneObject = static_cast<CompoundObject*>(landingActor.get());

      //m_debugDraw = new Box2DDraw;
      //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
      //m_debugDraw->attachTo(landingActor);
      //m_debugDraw->setWorld(Scales::c_physToStageScale, landingActor->GetWorld());
      //m_debugDraw->setPriority(1);


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

      //m_debugDraw = new Box2DDraw;
      //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
      //m_debugDraw->attachTo(freeSpaceActor);
      //m_debugDraw->setWorld(Scales::c_physToStageScale, freeSpaceActor->GetWorld());
      //m_debugDraw->setPriority(1);

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


      //m_debugDraw = new Box2DDraw;
      //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
      //m_debugDraw->attachTo(freeSpaceActor);
      //m_debugDraw->setWorld(Scales::c_physToStageScale, freeSpaceActor->GetWorld());
      //m_debugDraw->setPriority(1);
   }
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