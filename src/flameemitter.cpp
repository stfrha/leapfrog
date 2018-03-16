#include "LandingActor.h"

#include "static.h"

using namespace oxygine;
/*
LandingActor::LandingActor(Resources& gameResources) :
	m_world(NULL),
	m_zoomScale(0.40f),
	m_stageToViewPortScale(m_zoomScale * Scales::c_stageToViewPortScale),
	m_physToStageScale(1.0f)
{
	Point size = Point(1000.0, 500.0);
	setSize(size);

	m_world = new b2World(b2Vec2(0, 1.62));
	// m_world = new b2World(b2Vec2(0, 10));
	//		m_world = new b2World(b2Vec2(0, 0));

	// I should probably load resources that are uniuqe to the landing mode here

	// Position of frog in in the middle of the stage (500,250) meters in stage coordinates
	spLeapFrog leapFrog = new LeapFrog(gameResources, m_world, (Actor*)this, getSize() / 2, 1);

	//Vector2 frogPos = getStage()->getSize() / 2 - leapFrog->getSize() / 2;
	//leapFrog->setPosition(frogPos);

	addChild(leapFrog);
	m_leapfrog = leapFrog;

	spStatic ground = new Static(gameResources, m_world, RectF(465, 270, 25, 1));
	addChild(ground);

	spStatic platform = new Static(gameResources, m_world, RectF(520, 230, 25, 1));
	addChild(platform);

	spStatic pillar = new Static(gameResources, m_world, RectF(500, 305, 3, 100));
	addChild(pillar);

	setScale(m_stageToViewPortScale);


	//m_debugDraw = new Box2DDraw;
	//m_debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
	//m_debugDraw->attachTo(this);
	//m_debugDraw->setWorld(Scales::c_physToStageScale, m_world);
	//m_debugDraw->setPriority(1);
}


void LandingActor::doUpdate(const UpdateState& us)
{
	//in real project you should make steps with fixed dt, check box2d documentation
	m_world->Step(us.dt / 1000.0f, 6, 2);

	const Uint8* data = SDL_GetKeyboardState(0);
	
	if (data[SDL_SCANCODE_KP_PLUS])
	{
		m_zoomScale *= 1.1;
	}
	else if (data[SDL_SCANCODE_KP_MINUS])
	{
		m_zoomScale *= 0.9;
	}

	m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;

	if (data[SDL_SCANCODE_W])
	{
		m_leapfrog->fireMainBooster(true);
	}
	else
	{
		m_leapfrog->fireMainBooster(false);
	}

	if (data[SDL_SCANCODE_A])
	{
		m_leapfrog->fireSteeringBooster(-1);
	}
	else if (data[SDL_SCANCODE_D])
	{
		m_leapfrog->fireSteeringBooster(1);
	}
	else
	{
		m_leapfrog->fireSteeringBooster(0);
	}


	if (data[SDL_SCANCODE_0])
	{
		m_leapfrog->goToMode(LFM_RESET);
	}
	else if (data[SDL_SCANCODE_1])
	{
		m_leapfrog->goToMode(LFM_LANDING);
	}
	else if (data[SDL_SCANCODE_2])
	{
		m_leapfrog->goToMode(LFM_DEEP_SPACE);
	}
	else if (data[SDL_SCANCODE_3])
	{
		m_leapfrog->goToMode(LFM_REENTRY);
	}

	//update each body position on display
	b2Body* body = m_world->GetBodyList();
	while (body)
	{
		Actor* actor = (Actor*)body->GetUserData();
		b2Body* next = body->GetNext();
		if (actor)
		{
			const b2Vec2& pos = body->GetPosition();
			actor->setPosition(PhysDispConvert::convert(pos, Scales::c_physToStageScale));
			actor->setRotation(body->GetAngle());

			////remove fallen bodies
			//if (actor->getY() > getHeight() + 50)
			//{
			//	body->SetUserData(0);
			//	m_world->DestroyBody(body);

			//	actor->detach();
			//}
		}

		body = next;
	}

	setScale(m_stageToViewPortScale);

	// Find stagePos in viewPortCoord that makes frog at center
	Point vpSize = core::getDisplaySize();

	// Frog position in stage coordinates
	Vector2 frogPos = m_leapfrog->getPosition();

	Vector2 stagePos = Vector2(vpSize.x / 2, vpSize.y / 2) - frogPos * m_stageToViewPortScale;

	setPosition(stagePos);

}
*/


