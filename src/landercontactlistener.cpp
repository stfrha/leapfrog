#include "LanderContactListener.h"
#include "collisionentity.h"
#include "bullet.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "landingpad.h"
#include "asteroid.h"
#include "destroyable.h"
#include "blastemitter.h"

void LanderContactListener::InitContactListner(SceneActor* sceneActor)
{
   m_sceneActor = sceneActor;
}


void LanderContactListener::ContactHandler(b2Contact* contact, bool begin)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();


   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;
   Asteroid* asteroid = NULL;
   Actor* destroyable = NULL;
   b2Contact* destroyableContact;

   bool launchSiteLeftRest = false; // true means right leg rest
   bool leapfrogLeftFoot = false; // true means right foot

   if (eA == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = true;
   }

   if (eB == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = true;
   }

   if (eA == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = false;
   }

   if (eB == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = false;
   }

   if (eA == CET_LF_LEFT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = true;
   }

   if (eB == CET_LF_LEFT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = true;
   }

   if (eA == CET_LF_RIGHT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = false;
   }

   if (eB == CET_LF_RIGHT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = false;
   }

   if (eA == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_ASTEROID)
   {
      asteroid = (Asteroid*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_ASTEROID)
   {
      asteroid = (Asteroid*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_DESTROYABLE_OBJECT)
   {
      destroyable = (Actor*)contact->GetFixtureA()->GetBody()->GetUserData();
      destroyableContact = contact;
   }

   if (eB == CET_DESTROYABLE_OBJECT)
   {
      destroyable = (Actor*)contact->GetFixtureB()->GetBody()->GetUserData();
      destroyableContact = contact;
   }

   if (eA == CET_LANDING_PAD)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      landingPad = (LandingPad*)t->getParentObject();
   }

   if (eB == CET_LANDING_PAD)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      landingPad = (LandingPad*)t->getParentObject();
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

   if (asteroid && bullet)
   {
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      asteroid->hitByBullet(contact);
   }

   if (destroyable && bullet)
   {
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      KillDestroyable(destroyableContact, destroyable);
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

void LanderContactListener::KillDestroyable(b2Contact* contact, oxygine::Actor* actor)
{
   // Assume unshattered blast
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

      m_sceneActor->addMeToDeathList((ActorToDie*)actor);
   //}

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

