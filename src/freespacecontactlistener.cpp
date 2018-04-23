#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "asteroid.h"
#include "bullet.h"
#include "leapfrog.h"
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
      // Bullet hit asteroid
      bullet->hitAsteroid(contact);
      asteroid->hitByBullet(contact);
   }

   if (asteroid && shield)
   {
      // Asteroid hit shield
      shield->shieldHit(contact);
      asteroid->hitShield(contact);
   }

   // Gets here if shield no longer works
   if (asteroid && leapfrog)
   {
   }
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}



