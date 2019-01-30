#include "flameparticle.h"
#include "flamesmokeparticle.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"

using namespace oxygine;

FlameParticle::FlameParticle(
   Resources& gameResources,
   b2World* world,
   const b2Vec2& pos,
   const b2Vec2& vel,
   int lifetime,
   b2Vec2 impulseForce,
   float radius,
   int groupIndex) :
   m_gameResources(&gameResources),
   m_world(world)
{
   // If lifetime is zero, for instance if scale is zero,
   // We do not create any body
   if (lifetime == 0)
   {
      return;
   }

   setResAnim(gameResources.getResAnim("flame_particle"));
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

   body->ApplyLinearImpulse(impulseForce, b2Vec2(0.5, 0.5), true);

   m_body = body;
}

void FlameParticle::doUpdate(const oxygine::UpdateState& us)
{
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

