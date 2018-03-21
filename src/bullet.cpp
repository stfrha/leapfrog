#include "bullet.h"

#include "sceneactor.h"

using namespace oxygine;

Bullet::Bullet(
   oxygine::Resources& gameResources,
   SceneActor* sceneActor,
   b2World* world,
   const b2Vec2& pos,
   const float angle,
   float bulletSpeed,
   int m_lifetime,
   bool bouncy) : 
   m_bulletSpeed(bulletSpeed),
   m_sceneActor(sceneActor)
{
   setSize(0.75f, 0.75f);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));

   //spTween tranpTween = addTween(Actor::TweenAlpha(0), lifetime);
   //spTween scaleTween = addTween(Actor::TweenScale(0.25, 0.25), lifetime);

   //scaleTween->setDoneCallback(CLOSURE(this, &Bullet::atBulletDeath));

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   b2Body* body = world->CreateBody(&bodyDef);

   setUserData(body);

   b2CircleShape shape;
   shape.m_radius = 0.1f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 150.0f;
   fixtureDef.friction = 0.3f;
   fixtureDef.restitution = 0.5f;
   fixtureDef.filter.groupIndex = -1;

   body->CreateFixture(&fixtureDef);
   body->SetUserData(this);

   body->GetFixtureList()->SetUserData((CollisionEntity*)this);
   
   b2Vec2 vel = b2Vec2(m_bulletSpeed * cos(angle), m_bulletSpeed * sin(angle));

   body->SetLinearVelocity(vel);

   m_deathTime = getTimeMS() + m_lifetime;
}

CollisionEntityTypeEnum Bullet::getEntityType(void)
{
   return CET_BULLET;
}

void Bullet::killActor(void)
{
   atBulletDeath();
}



void Bullet::hitAsteroid(b2Contact* contact)
{
   m_sceneActor->addMeToDeathList((ActorToDie*)this);
}

void Bullet::doUpdate(const oxygine::UpdateState& us)
{
   if (us.time >= m_deathTime)
   {
      atBulletDeath();
   }
}

void Bullet::atBulletDeath(void)
{
   b2Body* myBody = (b2Body*)getUserData();

   myBody->GetWorld()->DestroyBody(myBody);

   this->detach();
}