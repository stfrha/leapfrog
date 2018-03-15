
#include <functional>
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include "physdispconvert.h"
#include "scales.h"

#include "mainactor.h"

using namespace oxygine;

MainActor::MainActor() :
	m_landingActor(NULL)
{
	//load xml file with resources definition
	m_gameResources.loadXML("res.xml");

	setSize(getStage()->getSize());

	spClipRectActor window = new ClipRectActor();

	window->setSize(getStage()->getSize() - Vector2(100, 100));
	window->setPosition(50.0f, 50.0f);
	addChild(window);

	spLandingActor landingActor = new LandingActor(m_gameResources);
	window->addChild(landingActor);

	addEventListener(oxygine::core::EVENT_SYSTEM, CLOSURE(this, &MainActor::onSystemEvent));

}

MainActor::~MainActor()
{
	//free previously loaded resources
	m_gameResources.free();
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

