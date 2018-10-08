#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "bodyuserdata.h"
#include "bullet.h"
#include "leapfrog.h"
#include "shield.h"
#include "hammer.h"
#include "breakableobject.h"
#include "gamestatus.h"


void FreeSpaceContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
}

void FreeSpaceContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
   CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   Shield* shield = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   Hammer* hammer = NULL;
   BreakableObject* breakable = NULL;

   if (eA == CET_LF_SHIELD)
   {
      shield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_LF_SHIELD)
   {
      shield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_BREAKABLE_OBJECT)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_BREAKABLE_OBJECT)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_LEAPFROG)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_LEAPFROG)
   {
      leapfrog = BodyUserData::getParentObjectOfType<LeapFrog*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_BULLET)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_BULLET)
   {
      bullet = BodyUserData::getParentObjectOfType<Bullet*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CET_HAMMER)
   {
      hammer = BodyUserData::getParentObjectOfType<Hammer*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CET_HAMMER)
   {
      hammer = BodyUserData::getParentObjectOfType<Hammer*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }


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
   if (hammer)
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



