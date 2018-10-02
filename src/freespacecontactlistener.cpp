#include "freespacecontactlistener.h"
#include "collisionentity.h"
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
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();

   Shield* shield = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   Hammer* hammer = NULL;
   BreakableObject* breakable = NULL;

   if (eA == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LF_SHIELD)
   {
      shield = (Shield*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_BREAKABLE_OBJECT)
   {
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureA()->GetBody()->GetUserData();

      breakable = (BreakableObject*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::breakableObject);
   }

   if (eB == CET_BREAKABLE_OBJECT)
   {
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureB()->GetBody()->GetUserData();

      breakable = (BreakableObject*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::breakableObject);
   }

   if (eA == CET_LEAPFROG)
   {
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureA()->GetBody()->GetUserData();

      leapfrog = (LeapFrog*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::leapfrog);
   }

   if (eB == CET_LEAPFROG)
   {
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureB()->GetBody()->GetUserData();

      leapfrog = (LeapFrog*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::leapfrog);
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
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureA()->GetBody()->GetUserData();

      hammer = (Hammer*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::steerableObject);
   }

   if (eB == CET_HAMMER)
   {
      CompoundObject* obj = (CompoundObject*)contact->GetFixtureB()->GetBody()->GetUserData();

      hammer = (Hammer*)obj->getParentWithBehaviour(CompoundObject::BehaviourEnum::steerableObject);
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
      //hammer->hitByAnything(contact);
   }
}


void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}



