#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "asteroid.h"
#include "bullet.h"
#include "leapfrog.h"

void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();

   Asteroid* asteroid = NULL;
   LeapFrog* lepafrog = NULL;
   Bullet* bullet = NULL;

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
      lepafrog = (LeapFrog*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LEAPFROG)
   {
      lepafrog = (LeapFrog*)contact->GetFixtureB()->GetBody()->GetUserData();
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

   if (asteroid && lepafrog)
   {
      // Bullet hit asteroid
      lepafrog->hitByAsteroid(contact);
      asteroid->hitByLepfrog(contact);
   }
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}

