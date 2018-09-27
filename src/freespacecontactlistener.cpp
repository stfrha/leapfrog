#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "bullet.h"
#include "leapfrog.h"
#include "shield.h"
#include "hammer.h"
#include "gamestatus.h"


void FreeSpaceContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
}

void FreeSpaceContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();

   Shield* shield = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   Hammer* hammer = NULL;

   if (eA == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   //if (eA == CET_ASTEROID)
   //{
   //   asteroid = (Asteroid*)contact->GetFixtureA()->GetBody()->GetUserData();
   //}

   //if (eB == CET_ASTEROID)
   //{
   //   asteroid = (Asteroid*)contact->GetFixtureB()->GetBody()->GetUserData();
   //}

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

   if (eA == CET_HAMMER)
   {
      hammer = (Hammer*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_HAMMER)
   {
      hammer = (Hammer*)contact->GetFixtureB()->GetBody()->GetUserData();
   }


   //if (asteroid && bullet)
   //{
   //   // Bullet hit asteroid
   //   bullet->hitAsteroid(contact);
   //   asteroid->hitByBullet(contact);
   //}

   //if (asteroid && shield)
   //{
   //   // Asteroid hit shield
   //   shield->shieldHit(contact, impulse);
   //   asteroid->hitShield(contact);
   //}

   //// Gets here if shield no longer works
   //if (asteroid && leapfrog)
   //{
   //   // Normally. body hits are prevented by shield
   //   // but if it a hard hit body may get contact
   //   // (from certain angles). In this case, if the 
   //   // shield is still active, we do not damage
   //   // leapfrog (by not calling hitAnything)
   //   if (g_GameStatus.getShield() <= 0.0f)
   //   {
   //      leapfrog->hitAnything(contact, impulse);
   //   }
   //}

   // Hammer hit anything
   if (hammer)
   {
      hammer->hitByAnything(contact);
   }
}


void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}



