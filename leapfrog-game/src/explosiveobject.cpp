
#include "explosiveobject.h"

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
   m_state(ExplosionState::idle)
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
   //m_breakAtDamage = node.attribute("breakDamage").as_int(0);
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

         m_state = ExplosionState::idle;
      }

      break;
   }

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

         int groupIndex = -g_GroupIndexSource.getNewGroupIndex();

         for (int i = 0; i < m_numOfParticles; i++) {
            float angle = (i / (float)m_numOfParticles) * 2.0f * MATH_PI;
            b2Vec2 rayDir(sinf(angle), cosf(angle));

            b2BodyDef bd;
            bd.type = b2_dynamicBody;
            bd.fixedRotation = true; // rotation not necessary
            bd.bullet = true; // prevent tunneling at high speed
            bd.linearDamping = 10; // drag due to moving through air
            bd.gravityScale = 0; // ignore gravity
            bd.position = center; // start at blast center
            bd.linearVelocity = m_blastPower * rayDir;
            b2Body* body = m_world->CreateBody(&bd);

            m_blastParticleBodies.push_back(body);

            b2CircleShape circleShape;
            circleShape.m_radius = 0.05; // very small

            b2FixtureDef fd;
            fd.shape = &circleShape;
            fd.density = 60 / (float)m_numOfParticles; // very high - shared across all particles
            fd.friction = 0; // friction not necessary
            fd.restitution = 0.99f; // high restitution to reflect off obstacles
            fd.filter.groupIndex = groupIndex; // particles should not collide with each other
            body->CreateFixture(&fd);
         }
      }
   }
}


void ExplosiveObject::triggerExplosion(void)
{
   doExplosion();
   m_state = ExplosionState::armExplosion;
}
