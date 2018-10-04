#include "landercontactlistener.h"
#include "compoundobject.h"
#include "collisionentity.h"
#include "bodyuserdata.h"
#include "bullet.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "landingpad.h"
#include "breakableobject.h"
#include "blastemitter.h"

void LanderContactListener::InitContactListner(SceneActor* sceneActor)
{
   m_sceneActor = sceneActor;
}

void LanderContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
   // We only check for Main body hits here since this is called more often than the
   //
   CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   LeapFrog* leapfrog = NULL;

   if (eA == CET_LEAPFROG)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_LEAPFROG)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }


   if (leapfrog)
   {
      leapfrog->hitAnything(contact, impulse);
   }
}








void LanderContactListener::ContactHandler(b2Contact* contact, bool begin)
{
   CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;
//   Asteroid* asteroid = NULL;
   CompoundObject* destroyable = NULL;
   BreakableObject* breakable = NULL;

   bool launchSiteLeftRest = false; // true means right leg rest
   bool leapfrogLeftFoot = false; // true means right foot

   if (eA == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureA()->GetBody()->GetUserData());

      launchSiteLeftRest = true;
   }

   if (eB == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureB()->GetBody()->GetUserData());

      launchSiteLeftRest = true;
   }

   if (eA == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureA()->GetBody()->GetUserData());

      launchSiteLeftRest = false;
   }

   if (eB == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureB()->GetBody()->GetUserData());

      launchSiteLeftRest = false;
   }

   if (eA == CET_LF_LEFT_FOOT)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());

      leapfrogLeftFoot = true;
   }

   if (eB == CET_LF_LEFT_FOOT)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());

      leapfrogLeftFoot = true;
   }

   if (eA == CET_LF_RIGHT_FOOT)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());

      leapfrogLeftFoot = false;
   }

   if (eB == CET_LF_RIGHT_FOOT)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());

      leapfrogLeftFoot = false;
   }


   if (eA == CET_BULLET)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_BULLET)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_BREAKABLE_OBJECT)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_BREAKABLE_OBJECT)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_DESTROYABLE_OBJECT)
   {
      destroyable = BodyUserData::getParentObjectOfType<CompoundObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_DESTROYABLE_OBJECT)
   {
      destroyable = BodyUserData::getParentObjectOfType<CompoundObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_LANDING_PAD)
   {
      landingPad = BodyUserData::getParentObjectOfType<LandingPad*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_LANDING_PAD)
   {
      landingPad = BodyUserData::getParentObjectOfType<LandingPad*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }


   if (launchSite && leapfrog && (leapfrogLeftFoot == launchSiteLeftRest))
   {
      if (begin)
      {
         launchSite->leapfrogFootTouch(contact, leapfrogLeftFoot);
      }
      else
      {
         launchSite->leapfrogFootLift(contact, leapfrogLeftFoot);
      }
   }

   if (landingPad && leapfrog)
   {
      if (begin)
      {
         landingPad->leapfrogFootTouch(contact, leapfrogLeftFoot);
      }
      else
      {
         landingPad->leapfrogFootLift(contact, leapfrogLeftFoot);
      }
   }

   if (breakable && bullet)
   {
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      breakable->hitByBullet(contact);
   }

   if (destroyable && bullet)
   {
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      destroyable->hitByBullet(contact);
   }
}

void LanderContactListener::BeginContact(b2Contact* contact)
{
   ContactHandler(contact, true);
}

void LanderContactListener::EndContact(b2Contact* contact)
{
   ContactHandler(contact, false);
}

//void LanderContactListener::KillDestroyable(b2Contact* contact, oxygine::Actor* actor)
//{
//   // Assume unshattered blast
//   int emitterLifetime = 150;
//   int particleLifetime = 500;
//   float particleDistance = 30.0f;
//   float particleSize = 0.75f;
//   float blastIntensity = 200.0f;
//
//   // Take damage
////   m_damage += 1;
//
//   //if (m_damage >= 4)
//   //{
//      emitterLifetime = 250;
//      particleLifetime = 500;
//      particleDistance = 60.0f;
//      particleSize = 0.9f;
//      blastIntensity = 300.0f;
//
//      m_sceneActor->addMeToDeathList((CompoundObject*)actor);
//   //}
//
//   b2WorldManifold m;
//   contact->GetWorldManifold(&m);
//
//   if (contact->GetManifold()->pointCount > 0)
//   {
//      spBlastEmitter blast = new BlastEmitter(
//         m_sceneActor->getResources(),
//         PhysDispConvert::convert(m.points[0], 1.0f),
//         blastIntensity,                                     // Intensity, particles / sec
//         emitterLifetime,                                    // Emitter Lifetime
//         particleLifetime,                                   // Particle lifetime
//         particleDistance,                                   // Particle distance
//         particleSize);                                      // Particle spawn size
//      blast->attachTo(m_sceneActor);
//   }
//}

