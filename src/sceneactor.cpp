#include "sceneactor.h"

#include "static.h"

using namespace oxygine;

SceneActor::SceneActor(Resources& gameResources, float zoomScale) :
   CompoundObject(),
   m_gameResources(&gameResources),
   m_world(NULL),
   m_zoomScale(zoomScale),
   m_stageToViewPortScale(m_zoomScale * Scales::c_stageToViewPortScale),
   m_physToStageScale(1.0f),
   m_panorateMode(PME_CENTER),
   m_externalControl(false),
   m_sceneType(STE_LANDING)
{
	Point size = Point(1000, 500);
	setSize(size);

	m_world = new b2World(b2Vec2(0.0f, 0.0f));

	setScale(m_stageToViewPortScale);
}

SceneActor::~SceneActor()
{
   delete m_world;
}

SceneTypeEnum SceneActor::getSceneType(void)
{
   return m_sceneType;
}

void SceneActor::setLeapfrog(spLeapFrog lf)
{
	m_leapfrog = lf;
}

b2World* SceneActor::GetWorld(void)
{
   return m_world;
}

Resources* SceneActor::getResources(void)
{
   return m_gameResources;
}

void SceneActor::setPanorateMode(PanorateModeEnum mode)
{
   m_panorateMode = mode;
}

void SceneActor::addMeToDeathList(CompoundObject* actor)
{
   if (std::find(m_deathList.begin(), m_deathList.end(), actor) != m_deathList.end())
   {
      // Attempted to add same object twice
   }
   else
   {
      m_deathList.push_back(actor);
   }
}

void SceneActor::takeControlOfLeapfrog(bool control)
{
   m_externalControl = control;
}

void SceneActor::doUpdate(const UpdateState& us)
{
	//in real project you should make steps with fixed dt, check box2d documentation
	m_world->Step(us.dt / 1000.0f, 6, 2);

   // Kill all actors registrated for death
   sweepKillList();

	const Uint8* data = SDL_GetKeyboardState(0);
	
	if (data[SDL_SCANCODE_KP_PLUS])
	{
		m_zoomScale *= 1.1f;
	}
	else if (data[SDL_SCANCODE_KP_MINUS])
	{
		m_zoomScale *= 0.9f;
	}

	m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;

   if (!m_externalControl)
   {
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
   }

   if (data[SDL_SCANCODE_RETURN])
   {
      m_leapfrog->fireGun(true);
   }
   else
   {
      m_leapfrog->fireGun(false);
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


   if (data[SDL_SCANCODE_KP_9])
   {
      m_leapfrog->fireReentryFlames(true);
   }
   else if (data[SDL_SCANCODE_KP_8])
   {
      m_leapfrog->fireReentryFlames(false);
   }

   if (data[SDL_SCANCODE_P])
   {
      m_leapfrog->dumpParts();
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

         // The sheild direction should not be set here, it
         // will change angle at collisions
         if (actor != (Actor*)(m_leapfrog->m_shield.get()))
         {
            actor->setRotation(body->GetAngle());
         }
         else
         {
            actor->setRotation(((Shield*)actor)->getAngle());
         }

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
	Vector2 frogPos = m_leapfrog->getMainActor()->getPosition();

   Vector2 wantedVpPos = Vector2(0.0f, 0.0f);
   
   if (m_panorateMode == PME_CENTER)
   {
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y / 2.0f);
   }
   else if (m_panorateMode == PME_TOP)
   { 
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y * 0.1f);
   }
   else if (m_panorateMode == PME_BOTTOM)
   {
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y * 0.9f);
   }
   else if (m_panorateMode == PME_TOP_LEFT)
   {
      wantedVpPos = Vector2(vpSize.x * 0.1f, vpSize.y * 0.1f);
   }
   
   if (m_panorateMode != PME_FIX)
   {
      Vector2 stagePos = wantedVpPos - frogPos * m_stageToViewPortScale;

      setPosition(stagePos);
   }
}

void SceneActor::sweepKillList(void)
{
   for (auto it = m_deathList.begin(); it != m_deathList.end(); ++it)
   {
      CompoundObject* a = (CompoundObject*)*it;
      a->killActor();
   }

   m_deathList.clear();
}

void SceneActor::createLeapFrog(Resources& gameResources)
{
   //spLeapFrog leapFrog = new LeapFrog(gameResources, this, m_world, (Actor*)this, getSize() / 2.0f);

   //addChild(leapFrog);
   //m_leapfrog = leapFrog;

   //// Here we should attach the shield to get it above the leapfrog
   //addChild(m_leapfrog->m_shield);
}