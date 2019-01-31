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
   CollisionEntity::CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntity::CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;
   CompoundObject* destroyableCO = NULL;
   Actor* destroyableActor = NULL;
   BreakableObject* breakable = NULL;

   if (eA == CollisionEntity::leapfrog)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::leapfrog)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CollisionEntity::bullet)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::bullet)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   // For destroyable, we want to kill the body that was hit, no the entire 
   // CompoundObject that it was a child to. However, we want also to remove 
   // the body (i.e. the shape) from the CompoundObject, so we still need the 
   // CO
   if (eA == CollisionEntity::destroyableObject)
   {
      destroyableActor = BodyUserData::getActor<Actor*>(contact->GetFixtureA()->GetBody()->GetUserData());
      destroyableCO = BodyUserData::getParentObjectOfType<CompoundObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::destroyableObject)
   {
      destroyableActor = BodyUserData::getActor<Actor*>(contact->GetFixtureB()->GetBody()->GetUserData());
      destroyableCO = BodyUserData::getParentObjectOfType<CompoundObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (bullet)
   {
      // Bullet hit something
      bullet->bulletHit(contact);

      if (breakable && bullet)
      {
         // Bullet hit breakable
         breakable->damageCollision(contact, 1.0f);
      }

      if (destroyableActor && bullet)
      {
         destroyableCO->removeShape(destroyableActor);
         m_sceneActor->addMeToDeathList(destroyableActor);
      }
   }

   if (leapfrog)
   {
      leapfrog->hitImpulse(contact, impulse);
   }
}

void LanderContactListener::LeapfrogFeetContactHandler(b2Contact* contact, bool begin)
{
   CollisionEntity::CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntity::CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   LeapFrog* leapfrog = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;

   bool launchSiteLeftRest = false; // true means right leg rest
   bool leapfrogLeftFoot = false; // true means right foot

   if (eA == CollisionEntity::lsLeftFootRest)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureA()->GetBody()->GetUserData());

      launchSiteLeftRest = true;
   }

   if (eB == CollisionEntity::lsLeftFootRest)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureB()->GetBody()->GetUserData());

      launchSiteLeftRest = true;
   }

   if (eA == CollisionEntity::lsRightFootRest)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureA()->GetBody()->GetUserData());

      launchSiteLeftRest = false;
   }

   if (eB == CollisionEntity::lsRightFootRest)
   {
      launchSite = BodyUserData::getParentObjectOfType<LaunchSite*>(contact->GetFixtureB()->GetBody()->GetUserData());

      launchSiteLeftRest = false;
   }

   if (eA == CollisionEntity::lfLeftFoot)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());

      leapfrogLeftFoot = true;
   }

   if (eB == CollisionEntity::lfLeftFoot)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());

      leapfrogLeftFoot = true;
   }

   if (eA == CollisionEntity::lfRightFoot)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());

      leapfrogLeftFoot = false;
   }

   if (eB == CollisionEntity::lfRightFoot)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());

      leapfrogLeftFoot = false;
   }

   if (eA == CollisionEntity::landingPad)
   {
      landingPad = BodyUserData::getParentObjectOfType<LandingPad*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::landingPad)
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
         landingPad->leapfrogFootTouch(contact, leapfrogLeftFoot, leapfrog );
         leapfrog->footOnLandingPad(true);
      }
      else
      {
         landingPad->leapfrogFootLift(contact, leapfrogLeftFoot, leapfrog);
         leapfrog->footOnLandingPad(false);
      }
   }

}

void LanderContactListener::BeginContact(b2Contact* contact)
{
   LeapfrogFeetContactHandler(contact, true);
}

void LanderContactListener::EndContact(b2Contact* contact)
{
   LeapfrogFeetContactHandler(contact, false);
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

