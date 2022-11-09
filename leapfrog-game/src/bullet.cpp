#include "bullet.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"
#include "blastemitter.h"

using namespace oxygine;

Bullet::Bullet(
   SceneActor* sceneActor,
   b2World* world,
   const b2Vec2& pos,
   const float angle,
   float impulseMagnitude,
   b2Vec2& craftSpeed,
   int lifetime,
   bool bouncy,
   int groupIndex) :
   m_sceneActor(sceneActor),
   m_groupIndex(groupIndex),
   m_firstUpdate(true),
   m_lifetime(lifetime)
{
   setSize(0.75f, 0.75f);
   setPosition(PhysDispConvert::convert(pos, 1.0f));
   setAnchor(Vector2(0.5f, 0.5f));
   setPriority(163);

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
   fixtureDef.filter.categoryBits = 1;
   fixtureDef.filter.maskBits = 65533;
   fixtureDef.filter.groupIndex = -groupIndex;

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

void Bullet::bulletHit(b2Contact* contact)
{
   if (m_sceneActor)
   {
      m_sceneActor->addMeToDeathList(this);
   }

   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;

   // Take damage
   //   m_damage += 1;

   //if (m_damage >= 4)
   //{
   emitterLifetime = 250;
   particleLifetime = 500;
   particleDistance = 60.0f;
   particleSize = 0.9f;
   blastIntensity = 300.0f;

   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      spBlastEmitter blast = new BlastEmitter(
         PhysDispConvert::convert(m.points[0], 1.0f),
         blastIntensity,                                     // Intensity, particles / sec
         emitterLifetime,                                    // Emitter Lifetime
         particleLifetime,                                   // Particle lifetime
         particleDistance,                                   // Particle distance
         particleSize);                                      // Particle spawn size
      blast->attachTo(m_sceneActor);
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

