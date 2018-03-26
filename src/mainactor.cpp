
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

	//spClipRectActor window = new ClipRectActor();

	//window->setSize(getStage()->getSize() - Vector2(100, 100));
	//window->setPosition(50.0f, 50.0f);
	//addChild(window);

	//spLandingActor landingActor = new LandingActor(m_gameResources);
	//window->addChild(landingActor);

 //  m_debugDraw = new Box2DDraw;
 //  m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
 //  m_debugDraw->attachTo(landingActor);
 //  m_debugDraw->setWorld(Scales::c_physToStageScale, landingActor->GetWorld());
 //  m_debugDraw->setPriority(1);

   changeToMode(STE_LANDING);
}

MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
}

void MainActor::changeToMode(SceneTypeEnum scene)
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

      spLandingActor landingActor = new LandingActor(m_gameResources);
      window->addChild(landingActor);

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

      spPlanetActor planetActor = new PlanetActor(m_gameResources);
      window->addChild(planetActor);

      //m_debugDraw = new Box2DDraw;
      //m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
      //m_debugDraw->attachTo(freeSpaceActor);
      //m_debugDraw->setWorld(Scales::c_physToStageScale, freeSpaceActor->GetWorld());
      //m_debugDraw->setPriority(1);
   }
}

void MainActor::doUpdate(const UpdateState& us)
{

}

void MainActor::onSystemEvent(Event* ev)
{
	// The following never worked, now it will work even 
	// less since there is a clipped window between this and the main actor

	//SDL_Event *event = (SDL_Event*)ev->userData;

	//if (event->type != SDL_KEYDOWN)
	//	return;

	//if (event->key.keysym.sym == SDL_SCANCODE_F1)
	//{
	//	// Restart MainActor and start LandingActor
	//	spLandingActor landingActor = new LandingActor(gameResources);
	//	addChild(landingActor);
	//}

	//if (event->key.keysym.sym == SDL_SCANCODE_F2)
	//{
	//	// Restart MainActor and start LandingActor
	//	spActor actor = getFirstChild();

	//	while (actor)
	//	{
	//		spActor next = actor->getNextSibling();
	//		if (actor != NULL)
	//		{
	//			actor->detach();
	//		}
	//		actor = next;
	//	}
	//}
}

