

#include "ReentryCompositeActor.h"


using namespace oxygine;

ReentryCompositeActor::ReentryCompositeActor(Resources& gameResources) :
   m_burnedLastUpdate(false),
   m_state(RES_INITITAL)
{
   //// I should probably load resources that are uniuqe here
   //setPanorateMode(PME_FIX);

   //createLeapFrog(gameResources);

   //m_leapfrog->goToMode(LFM_ORBIT);
   //m_leapfrog->goToEnvironment(ENV_ORBIT);

   m_planetActor = new PlanetActor(gameResources);
   m_planetActor->setAnchor(0.5f, 0.5f);
   m_planetActor->setPosition(-30.0f, 20.0f);
//   m_planetActor->setScale(0.075f);
   m_planetActor->setScale(0.3f);
   m_planetActor->attachTo(this);

   // Build closeup of Leapfrog
   spColorRectSprite frame = new ColorRectSprite();
   frame->setSize(304.0f, 154.0f);
//   frame->setAnchor(0.5f, 0.5f);
   frame->setPosition(548.0f, 0.0f);
   frame->attachTo(this);

   spClipRectActor clip = new ClipRectActor();
   clip->setSize(300.0f, 150.0f);
   clip->setPosition(2.0f, 2.0f);
   clip->attachTo(frame);

   spSprite test = new Sprite();
   test->setResAnim(gameResources.getResAnim("starfield"));
   test->setSize(600.0f, 300.0f);
   test->setPosition(-300.0f, 0.0f);
   test->attachTo(clip);

   m_lfStage = new Actor();
   m_lfStage->setSize(300.0f, 150.0f);
   m_lfStage->setAnchor(0.5f, 0.5f);
   m_lfStage->setPosition(-50.0f, -20.0f);
   m_lfStage->attachTo(clip);

   m_world = new b2World(b2Vec2(0.0f, 0.0f));

//   spLeapFrog lf = new LeapFrog(gameResources, NULL,  m_world, (Actor*)lfStage.get(), Vector2(150.0f, 75.0f));
   m_leapFrog = new LeapFrog(gameResources, NULL, m_world, m_lfStage.get(), Vector2(150.0f, 75.0f));
   m_leapFrog->attachTo(m_lfStage);
   m_leapFrog->initLeapfrog(LFM_DEEP_SPACE, MATH_PI / 2.0f);
//   m_leapFrog->initMode(LFM_REENTRY);

   m_lfStage->setScale(4.0f);



   test->addTween(TweenPosition(0.0f, 0.0f), 5000, 1, false, 0, Tween::ease_outQuad);

   // Take back after debug
   spTween waitTween = addTween(TweenDummy(), 2000);
   waitTween->setDoneCallback(CLOSURE(this, &ReentryCompositeActor::onWaitDone));

   // Remove after debug
//   m_planetActor->orbitEstablished();
}


void ReentryCompositeActor::doUpdate(const UpdateState& us)
{
   //in real project you should make steps with fixed dt, check box2d documentation
   m_world->Step(us.dt / 1000.0f, 6, 2);

   if (m_state == RES_WAIT_FOR_BREAK)
   {
      const Uint8* data = SDL_GetKeyboardState(0);

      if (data[SDL_SCANCODE_W])
      {
         if (!m_burnedLastUpdate)
         {
            m_planetActor->startBurn();
            m_leapFrog->fireMainBooster(true);
            m_burnedLastUpdate = true;
         }
      }
      else
      {
         if (m_burnedLastUpdate)
         {
            m_planetActor->stopBurn();
            m_leapFrog->fireMainBooster(false);
            m_burnedLastUpdate = false;

            // Here the burn is complete
            float burnAmount = m_planetActor->getBurnAmount();

            if ((burnAmount > 0.6f) && (burnAmount < 0.8f))
            {
               m_planetActor->startReentry();
               m_reentryStartTime = us.time;
               m_state = RES_REENTRY;
            }
         }
      }
   }
   else if (m_state == RES_REENTRY)
   {
      m_leapFrog->setHoldAngle(m_planetActor->getReentryLeapfrogAngle(us.time - m_reentryStartTime) - MATH_PI / 2.0f);
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
         if (actor != (Actor*)(m_leapFrog->m_shield.get()))
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
}

void ReentryCompositeActor::onWaitDone(Event* event)
{
   m_leapFrog->fireMainBooster(true);
   m_leapFrog->setHoldAngle(MATH_PI / 2.0f);
   spTween centerTween = m_lfStage->addTween(TweenPosition(150.0f, 50.0f), 3000, 1, false, 0, Tween::ease_outQuint);
   centerTween->setDoneCallback(CLOSURE(this, &ReentryCompositeActor::onCenterDone));
}

void ReentryCompositeActor::onCenterDone(Event* event)
{
   spTween t = addTween(TweenDummy(), 250);
   t->setDoneCallback(CLOSURE(this, &ReentryCompositeActor::onBurnDone));
}

void ReentryCompositeActor::onBurnDone(Event* event)
{
   m_leapFrog->fireMainBooster(false);
   m_leapFrog->fireSteeringBooster(-1);
   m_leapFrog->setHoldAngle(-MATH_PI / 2.0f);
   spTween t = addTween(TweenDummy(), 2000);
   t->setDoneCallback(CLOSURE(this, &ReentryCompositeActor::onRotateDone));
}

void ReentryCompositeActor::onRotateDone(Event* event)
{
   m_leapFrog->fireSteeringBooster(0);
   m_leapFrog->goToMode(LFM_REENTRY);
   m_leapFrog->setHoldAngle(-MATH_PI / 2.0f);
   spTween t = addTween(TweenDummy(), 4500);
   t->setDoneCallback(CLOSURE(this, &ReentryCompositeActor::onModeDone));
}

void ReentryCompositeActor::onModeDone(Event* event)
{
   m_planetActor->orbitEstablished();
   m_state = RES_WAIT_FOR_BREAK;
}
