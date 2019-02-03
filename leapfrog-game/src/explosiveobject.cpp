
#include "explosiveobject.h"

#include "bodyuserdata.h"
#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"

#include "groupindexsource.h"

using namespace oxygine;
using namespace pugi;

ExplosiveObject::ExplosiveObject(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_gameResource(&gameResources),
   m_sceneActor(sceneParent),
   m_world(world),
   m_numOfParticles(20),
   m_particlesRadius(0.5f),
   m_blastPower(5000.0f),
   m_state(ExplosionState::idle),
   m_explosionAnimation(NULL)
{
   readExplosiveObjectNode(root.child("behaviour").child("explosiveObjectProperties"));
     
   initCompoundObjectParts(
      gameResources,
      sceneParent,
      parentObject,
      world,
      pos,
      root,
      "",
      groupIndex);

   attachTo(m_sceneActor);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
   {
      FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

      setAllBodiesToBounding(spaceActor);
   }
}

void ExplosiveObject::readExplosiveObjectNode(const pugi::xml_node& node)
{
   m_numOfParticles = node.attribute("numOfParticles").as_int(0);
   m_particlesRadius = node.attribute("particleRadius").as_float(0.0f);
   m_blastPower = node.attribute("blastPower").as_float(0.0f);
}


void ExplosiveObject::doUpdate(const oxygine::UpdateState& us)
{
   const Uint8* data = SDL_GetKeyboardState(0);

   switch (m_state)
   {
   case ExplosionState::idle:

      if (data[SDL_SCANCODE_B])
      {
         triggerExplosion();
      }

      break;

   case ExplosionState::armExplosion:

      // TODO: Start explosion animation here

      // By trigger an explosion using the armExplosion state,
      // we make sure that it is started during a step, where
      // it is ok to remove actors and kill bodies.
      doExplosion();

      m_blastTime = us.time;
      m_state = ExplosionState::waitForExplosionEnd;
      break;

   case ExplosionState::waitForExplosionEnd:

      if (us.time > m_blastTime + 500)
      {
         for (auto it = m_blastParticleBodies.begin(); it != m_blastParticleBodies.end(); ++it)
         {
            m_world->DestroyBody(*it);
         }

         if (m_explosionAnimation != NULL)
         {
            m_explosionAnimation->detach();
         }

         m_state = ExplosionState::idle;
      }

      break;
   }

}

void ExplosiveObject::startAnimation(b2Vec2 pos)
{
   oxygine::ResAnim* resAnim = m_gameResource->getResAnim("explosion_ani");

   m_explosionAnimation = new Sprite();
   m_explosionAnimation->setAnchor(0.5f, 0.5f);
   m_explosionAnimation->setPosition(PhysDispConvert::convert(pos, 1.0f));
   m_sceneActor->addChild(m_explosionAnimation);
   spTween animTween = m_explosionAnimation->addTween(TweenAnim(resAnim, 0, 63), 500);
   m_explosionAnimation->setSize(200.0f, 200.0f);
   m_explosionAnimation->setPriority(164);
}

void ExplosiveObject::doExplosion(void)
{
   m_blastParticleBodies.clear();

   if (m_shapes.size() > 0)
   {
      b2Body* myBody = ActorUserData::getBody(m_shapes[0]->getUserData());

      if (myBody)
      {
         b2Vec2 center = myBody->GetPosition();

         startAnimation(center);

         // Now that we have saved the position, it is safe to remove
         // the body.
         m_world->DestroyBody(myBody);
         m_shapes[0]->detach();
         
         int groupIndex = -g_GroupIndexSource.getNewGroupIndex();

         for (int i = 0; i < m_numOfParticles; i++) {
            float angle = (i / (float)m_numOfParticles) * 2.0f * MATH_PI;
            b2Vec2 rayDir(sinf(angle), cosf(angle));

            b2BodyDef bd;
            bd.type = b2_dynamicBody;
            bd.fixedRotation = true; // rotation not necessary
            bd.bullet = true; // prevent tunneling at high speed
            bd.linearDamping = 7; // drag due to moving through air
            bd.gravityScale = 0; // ignore gravity
            bd.position = center; // start at blast center
            bd.linearVelocity = m_blastPower * rayDir;
            b2Body* body = m_world->CreateBody(&bd);

            m_blastParticleBodies.push_back(body);

            b2CircleShape circleShape;
            circleShape.m_radius = m_particlesRadius; // very small

            b2FixtureDef fd;
            fd.shape = &circleShape;
            fd.density = 60 / (float)m_numOfParticles; // very high - shared across all particles
            fd.friction = 0; // friction not necessary
            fd.restitution = 0.99f; // high restitution to reflect off obstacles
            fd.filter.groupIndex = groupIndex; // particles should not collide with each other

            BodyUserData* bud = new BodyUserData();
            bud->m_actor = this;
            bud->m_collisionType = CollisionEntity::blastParticle;

            fd.userData = bud;

            body->CreateFixture(&fd);

            body->SetUserData(bud);
         }
      }
   }
}


void ExplosiveObject::triggerExplosion(void)
{
   m_state = ExplosionState::armExplosion;
}
