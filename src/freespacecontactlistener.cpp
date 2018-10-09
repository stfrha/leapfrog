#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "bodyuserdata.h"
#include "bullet.h"
#include "leapfrog.h"
#include "shield.h"
#include "steerableobject.h"
#include "breakableobject.h"
#include "gamestatus.h"


void FreeSpaceContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
}

void FreeSpaceContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
   CollisionEntity::CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntity::CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   Shield* shield = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   SteerableObject* steerableObject = NULL;
   BreakableObject* breakable = NULL;

   if (eA == CollisionEntity::lfShield)
   {
      shield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::lfShield)
   {
      shield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

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

   //if (eA == CET_HAMMER)
   //{
   //   hammer = BodyUserData::getParentObjectOfType<Hammer*>(contact->GetFixtureA()->GetBody()->GetUserData());
   //}

   //if (eB == CET_HAMMER)
   //{
   //   hammer = BodyUserData::getParentObjectOfType<Hammer*>(contact->GetFixtureB()->GetBody()->GetUserData());
   //}


   if (breakable && bullet)
   {
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      breakable->hitByBullet(contact);
   }

   if (breakable && shield)
   {
      // Asteroid hit shield
      shield->shieldHit(contact, impulse);
      breakable->hitShield(contact);
   }

   // Gets here if shield no longer works
   if (breakable && leapfrog)
   {
      // Normally. body hits are prevented by shield
      // but if it a hard hit body may get contact
      // (from certain angles). In this case, if the 
      // shield is still active, we do not damage
      // leapfrog (by not calling hitAnything)
      if (g_GameStatus.getShield() <= 0.0f)
      {
         leapfrog->hitAnything(contact, impulse);
      }
   }

   // Hammer hit anything
   if (steerableObject)
   {
      //hammer->hitByAnything(contact);
   }
}


void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}



