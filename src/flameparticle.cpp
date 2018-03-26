#include "flameparticle.h"
#include "smokeparticle.h"

using namespace oxygine;

FlameParticle::FlameParticle(
   Resources& gameResources,
   b2World* world,
   const b2Vec2& pos,
   int lifetime,
   b2Vec2 impulseForce,
   float radius) :
   m_gameResources(&gameResources)
{
   setResAnim(gameResources.getResAnim("flame_particle"));
   setSize(radius, radius);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));

   spTween tranpTween = addTween(Actor::TweenAlpha(0), lifetime);
   spTween scaleTween = addTween(Actor::TweenScale(0.25, 0.25), lifetime);

   scaleTween->setDoneCallback(CLOSURE(this, &FlameParticle::atParticleDeath));

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   b2Body* body = world->CreateBody(&bodyDef);

   setUserData(body);

   b2CircleShape shape;
   shape.m_radius = 0.5f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 3.0f;
   fixtureDef.friction = 0.3f;
   fixtureDef.restitution = 0.5f;
   fixtureDef.filter.groupIndex = -1;

   body->CreateFixture(&fixtureDef);
   body->SetUserData(this);

   body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   body->ApplyLinearImpulse(impulseForce, b2Vec2(0.5, 0.5), true);
}

CollisionEntityTypeEnum FlameParticle::getEntityType(void)
{
   return CET_FLAME_PARTICLE;
}

void FlameParticle::doUpdate(const oxygine::UpdateState& us)
{
}

void FlameParticle::atParticleDeath(oxygine::Event* event)
{
   // Create a smoke particle
   //spSmokeParticle smoke = new SmokeParticle(m_gameResources, getPosition(), 1000.0f, 0.0f, 10.0f);
   //smoke->attachTo(getParent());


   // Now I'm suppose to comit suicide. How to deregister me from actor and world?
   b2Body* myBody = (b2Body*)getUserData();

   myBody->GetWorld()->DestroyBody(myBody);

   this->detach();



}