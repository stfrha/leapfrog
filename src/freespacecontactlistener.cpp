#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "asteroid.h"
#include "bullet.h"
#include "leapfrog.h"
#include "blastemitter.h"
#include "shield.h"

void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();

   Shield* shield = NULL;
   Asteroid* asteroid = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;

   if (eA == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_ASTEROID)
   {
      asteroid = (Asteroid*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_ASTEROID)
   {
      asteroid = (Asteroid*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_LEAPFROG)
   {
      leapfrog = (LeapFrog*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LEAPFROG)
   {
      leapfrog = (LeapFrog*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureB()->GetBody()->GetUserData();
   }


   if (asteroid && bullet)
   {
      int emitterLifetime = 150;
      int particleLifetime = 500;
      float particleDistance = 15.0f;
      float particleSize = 0.75f;
      float blastIntensity = 200.0f;

      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      
      if (asteroid->hitByBullet(contact))
      {
         emitterLifetime = 250;
         particleLifetime = 500;
         particleDistance = 30.0f;
         particleSize = 0.9f;
         blastIntensity = 300.0f;
      }

      // Sparkle for a while there

      b2WorldManifold m;
      contact->GetWorldManifold(&m);

      if (contact->GetManifold()->pointCount > 0)
      {
         spBlastEmitter blast = new BlastEmitter(
            m_sceneActor->getResources(), 
            PhysDispConvert::convert(m.points[0], 1.0f), 
            blastIntensity,                                     // Intensity, particles / sec
            emitterLifetime,                                    // Emitter Lifetime
            particleLifetime,                                   // Particle lifetime
            particleDistance,                                   // Particle distance
            particleSize);                                      // Particle spawn size
         blast->attachTo(m_sceneActor);
      }
   }

   if (asteroid && shield)
   {
      // Asteroid hit shield
      // leapfrog->hitByAsteroid(contact);
      // asteroid->hitByLepfrog(contact);

      b2WorldManifold m;
      contact->GetWorldManifold(&m);

      if (contact->GetManifold()->pointCount > 0)
      {
         b2Vec2 collVec = m.points[0] - shield->m_body->GetPosition();

         float angle = atan2(collVec.y, collVec.x) + MATH_PI / 2.0f;

         shield->setAngle(angle);
         shield->shieldHit();

         int emitterLifetime = 250;
         int particleLifetime = 350;
         float particleDistance = 40.0f;
         float particleSize = 0.9f;

         spBlastEmitter blast = new BlastEmitter(
            m_sceneActor->getResources(),
            PhysDispConvert::convert(m.points[0], 1.0f),
            150.0f,                                             // Intensity, particles / sec
            emitterLifetime,                                    // Emitter Lifetime
            particleLifetime,                                   // Particle lifetime
            particleDistance,                                   // Particle distance
            particleSize);                                      // Particle spawn size
         blast->attachTo(m_sceneActor);

      }
   }

   // Gets here if shield no longer works
   if (asteroid && leapfrog)
   {
//      // Bullet hit asteroid
//      leapfrog->hitByAsteroid(contact);
//      asteroid->hitByLepfrog(contact);
//
//      b2WorldManifold m;
//      contact->GetWorldManifold(&m);
//
//      if (contact->GetManifold()->pointCount > 0)
//      {
//         Vector2 collPos = PhysDispConvert::convert(m.points[0], 1.0f);
//
//         Vector2 collVector = collPos - leapfrog->getPosition();
//
//
////         float angle = atan2(collVector.y , collVector.x);
//
//         // Randomise value between -m_emitterWidth / 2 and m_emitterWidth / 2
//         float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f / MATH_PI));
//
//         leapfrog->m_shield->m_body->SetTransform(leapfrog->m_shield->m_body->GetPosition(), angle);
//
//         //spShield shield = new Shield(m_sceneActor->getResources(), leapfrog->getPosition(), angle);
//         //shield->attachTo(leapfrog);
//      }
   }
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}

void FreeSpaceContactListener::setSceneActor(SceneActor* actor)
{
   m_sceneActor = actor;

}


