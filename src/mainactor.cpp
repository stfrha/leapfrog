
#include <functional>
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include "physdispconvert.h"
#include "scales.h"

#include "mainactor.h"

using namespace oxygine;

MainActor::MainActor()
{
	//load xml file with resources definition
	m_gameResources.loadXML("res.xml");

	setSize(getStage()->getSize());

   startScene(STE_LANDING);
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

      spLandingActor landingActor = new LandingActor(m_gameResources, "landing_scene.xml");
      window->addChild(landingActor);

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

      spFreeSpaceActor freeSpaceActor = new FreeSpaceActor(m_gameResources);
      window->addChild(freeSpaceActor);

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

      spReentryCompositeActor reentryActor = new ReentryCompositeActor(m_gameResources);
      window->addChild(reentryActor);

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
