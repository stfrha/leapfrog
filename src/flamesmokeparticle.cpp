#include "flamesmokeparticle.h"
#include "smokeparticle.h"

using namespace oxygine;

FlameSmokeParticle::FlameSmokeParticle(
   Resources& gameResources,
   b2World* world,
   const b2Vec2& pos,
   const b2Vec2& vel,
   int lifetime,
   float radius) :
   m_gameResources(&gameResources)
{
   setResAnim(gameResources.getResAnim("white_circle"));
   setSize(radius, radius);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));
   setPriority(106);
   setAlpha(32);
   setScale(0.1f);

   spTween tranpTween = addTween(Actor::TweenAlpha(0), lifetime);
   spTween scaleTween = addTween(Actor::TweenScale(2.0, 2.0), lifetime);

   scaleTween->setDoneCallback(CLOSURE(this, &FlameSmokeParticle::atParticleDeath));

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   m_body = world->CreateBody(&bodyDef);

   setUserData(m_body);

   b2CircleShape shape;
   shape.m_radius = 0.5f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 3.0f;
   fixtureDef.friction = 0.3f;
   fixtureDef.restitution = 0.5f;
   fixtureDef.filter.categoryBits = 8192;
   fixtureDef.filter.maskBits = 56459;

   m_body->CreateFixture(&fixtureDef);
   m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   // Give particle the same speed as the emitter body
   // but with a slightly different trajectory
   b2Vec2 dir = vel;
   float velocity = dir.Normalize();
   float angle = atan2(dir.y, dir.x);

   // Randomise angle to get spread
   float ra = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f / 180.0f * MATH_PI))) - (15.0f / 180.0f * MATH_PI);

   angle += ra;

   b2Vec2 newVel = b2Vec2(velocity * cos(angle), velocity * sin(angle));

   m_originalDirection = PhysDispConvert::convert(newVel, 1.0f);
   m_originalDirection.normalize();

   m_body->SetLinearVelocity(newVel);

   m_antiGravForce = -20.0f * world->GetGravity();

   m_body->ApplyForce(m_antiGravForce, b2Vec2(0.5, 0.5), true);
}

CollisionEntityTypeEnum FlameSmokeParticle::getEntityType(void)
{
   return CET_FLAME_PARTICLE;
}

void FlameSmokeParticle::doUpdate(const UpdateState& us)
{
   b2Vec2 vel = m_body->GetLinearVelocity();
   Vector2 v = PhysDispConvert::convert(vel, 1.0f);
   float velAtOriginalDir = m_originalDirection.dot(v);

   b2Vec2 redForce = -velAtOriginalDir * 100.0f * PhysDispConvert::convert(m_originalDirection, 1.0f);

   m_body->ApplyForce(m_antiGravForce + redForce, b2Vec2(0.5, 0.5), true);
}

void FlameSmokeParticle::atParticleDeath(Event* event)
{
   // Now I'm suppose to comit suicide. How to deregister me from actor and world?
   b2Body* myBody = (b2Body*)getUserData();

   myBody->GetWorld()->DestroyBody(myBody);

   this->detach();
}