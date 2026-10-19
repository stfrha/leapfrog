#include "flameparticle.h"
#include "flamesmokeparticle.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"
#include "graphicresources.h"

using namespace oxygine;

FlameParticle::FlameParticle(
   b2World* world,
   const b2Vec2& pos,
   const b2Vec2& vel,
   int lifetime,
   b2Vec2 impulseForce,
   float radius,
   int groupIndex) :
   m_world(world),
   m_body(NULL),
   m_startVel(0.0f, 0.0f),
   m_groupIndex(groupIndex),
   m_strayReported(false)
{
   // If lifetime is zero, for instance if scale is zero,
   // We do not create any body
   if (lifetime == 0)
   {
      return;
   }

   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim("flame_particle"));
   setSize(radius, radius);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));
   setPriority(106);

   spTween tranpTween = addTween(Actor::TweenAlpha(0), lifetime);
   spTween scaleTween = addTween(Actor::TweenScale(0.25, 0.25), lifetime);

   scaleTween->setDoneCallback(CLOSURE(this, &FlameParticle::atParticleDeath));

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   b2Body* body = world->CreateBody(&bodyDef);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = NULL;

   setUserData(aud);

   b2CircleShape shape;
   shape.m_radius = 0.5f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 3.0f;
   fixtureDef.friction = 0.3f;
   fixtureDef.restitution = 0.5f;
   fixtureDef.filter.categoryBits = 8;
   fixtureDef.filter.maskBits = 65525;
   fixtureDef.filter.groupIndex = -groupIndex;

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = this;
   bud->m_collisionType = CollisionEntity::flameParticle;

   fixtureDef.userData = bud;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(bud);

   // Give particle the same speed as the emitter body
   body->SetLinearVelocity(vel);

   // At the centre: the point is in world coordinates, so the old fixed
   // (0.5, 0.5) also set the particle spinning
   body->ApplyLinearImpulse(impulseForce, body->GetWorldCenter(), true);

   m_body = body;
   m_startVel = body->GetLinearVelocity();
}

void FlameParticle::doUpdate(const oxygine::UpdateState& us)
{
   // TEMPORARY DIAGNOSTIC (stray flame particles): report a particle whose
   // velocity changes noticeably after it was created, and what it is
   // touching then. Particles move in a straight line otherwise (no
   // gravity in space), so a change means something pushed it.
   if ((m_body == NULL) || m_strayReported)
   {
      return;
   }

   b2Vec2 vel = m_body->GetLinearVelocity();
   b2Vec2 change = vel - m_startVel;

   if (change.Length() > 5.0f)
   {
      m_strayReported = true;

      static int s_strays = 0;
      s_strays++;
      if (s_strays > 40)
      {
         return;
      }

      std::string touching;
      for (b2ContactEdge* ce = m_body->GetContactList(); ce; ce = ce->next)
      {
         b2Fixture* other = (ce->contact->GetFixtureA()->GetBody() == m_body) ?
            ce->contact->GetFixtureB() : ce->contact->GetFixtureA();
         char buf[96];
         snprintf(buf, sizeof(buf), " [type %d group %d touching %d]",
            (int)BodyUserData::getCollisionType(other->GetUserData()),
            (int)other->GetFilterData().groupIndex,
            (int)ce->contact->IsTouching());
         touching += buf;
      }

      b2Vec2 pos = m_body->GetPosition();
      logs::messageln("STRAY #%d group %d at (%.1f, %.1f): vel (%.1f, %.1f) -> (%.1f, %.1f), contacts:%s",
         s_strays, -m_groupIndex, pos.x, pos.y,
         m_startVel.x, m_startVel.y, vel.x, vel.y,
         touching.empty() ? " none" : touching.c_str());
   }
}

void FlameParticle::atParticleDeath(oxygine::Event* event)
{
   // Create three smoke particles

   //spFlameSmokeParticle p1 = new FlameSmokeParticle(
   //   *m_gameResources, 
   //   m_world, 
   //   m_body->GetPosition(), 
   //   m_body->GetLinearVelocity(), 
   //   1500, 
   //   2.0f);

   //p1->attachTo(getParent());

   //spFlameSmokeParticle p2 = new FlameSmokeParticle(
   //   *m_gameResources,
   //   m_world,
   //   m_body->GetPosition(),
   //   m_body->GetLinearVelocity(),
   //   1500,
   //   2.0f);

   //p2->attachTo(getParent());

   //spFlameSmokeParticle p3 = new FlameSmokeParticle(
   //   *m_gameResources,
   //   m_world,
   //   m_body->GetPosition(),
   //   m_body->GetLinearVelocity(),
   //   1500,
   //   2.0f);

   //p3->attachTo(getParent());

   //spSmokeParticle smoke = new SmokeParticle(m_gameResources, getPosition(), 1000.0f, 0.0f, 10.0f);
   //smoke->attachTo(getParent());


   // Now I'm suppose to comit suicide. How to deregister me from actor and world?
   b2Body* myBody = ActorUserData::getBody(getUserData());

   myBody->GetWorld()->DestroyBody(myBody);

   this->detach();
}

