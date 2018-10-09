#include "bullet.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"

using namespace oxygine;

Bullet::Bullet(
   oxygine::Resources& gameResources,
   SceneActor* sceneActor,
   b2World* world,
   const b2Vec2& pos,
   const float angle,
   float impulseMagnitude,
   b2Vec2& craftSpeed,
   int lifetime,
   bool bouncy) : 
   m_sceneActor(sceneActor),
   m_firstUpdate(true),
   m_lifetime(lifetime)
{
   setSize(0.75f, 0.75f);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));
   setPriority(163);

   //spTween tranpTween = addTween(Actor::TweenAlpha(0), lifetime);
   //spTween scaleTween = addTween(Actor::TweenScale(0.25, 0.25), lifetime);

   //scaleTween->setDoneCallback(CLOSURE(this, &Bullet::atBulletDeath));

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   b2Body* body = world->CreateBody(&bodyDef);

   b2CircleShape shape;
   shape.m_radius = 0.5f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 3.0f;
   fixtureDef.friction = 0.3f;
   fixtureDef.restitution = 0.5f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 65215;

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = this;
   bud->m_collisionType = CollisionEntity::bullet;

   fixtureDef.userData = bud;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = NULL;

   setUserData(aud);


   b2Vec2 vel = b2Vec2(impulseMagnitude * cos(angle), impulseMagnitude * sin(angle));

//   vel += craftSpeed;

   body->SetLinearVelocity(craftSpeed);

   body->ApplyLinearImpulseToCenter(vel, true);

}

//void Bullet::killActor(void)
//{
//   atBulletDeath();
//}



void Bullet::hitAsteroid(b2Contact* contact)
{
   if (m_sceneActor)
   {
      m_sceneActor->addMeToDeathList(this);
   }
}

void Bullet::doUpdate(const oxygine::UpdateState& us)
{
   if (m_firstUpdate)
   {
      m_deathTime = us.time + m_lifetime;
      m_firstUpdate = false;
   }

   if (us.time >= m_deathTime)
   {
      if (m_sceneActor)
      {
         m_sceneActor->addMeToDeathList(this);
      }
   }
}

void Bullet::killAllChildBodies(void)
{
   b2Body* myBody = ActorUserData::getBody(getUserData());

   if (myBody)
   {
      myBody->GetWorld()->DestroyBody(myBody);

   }
}

void Bullet::atBulletDeath(void)
{
   b2Body* myBody = ActorUserData::getBody(getUserData());

   myBody->GetWorld()->DestroyBody(myBody);

   this->detach();
}