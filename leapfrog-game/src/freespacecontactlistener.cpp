#include "freespacecontactlistener.h"
#include "collisionentity.h"
#include "bodyuserdata.h"
#include "bullet.h"
#include "leapfrog.h"
#include "shield.h"
#include "steerableobject.h"
#include "breakableobject.h"
#include "explosiveobject.h"
#include "gamestatus.h"


void FreeSpaceContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
}

void FreeSpaceContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
   CollisionEntity::CollisionEntityTypeEnum eA = BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData());
   CollisionEntity::CollisionEntityTypeEnum eB = BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData());

   Shield* shield1 = NULL;
   Shield* shield2 = NULL;
   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   SteerableObject* steerableObject = NULL;
   BreakableObject* breakable = NULL;
   ExplosiveObject* explosive = NULL;
   bool flameParticle = false;
   bool blastParticle = false;

   if (eA == CollisionEntity::shield)
   {
      Shield* tmpShield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureA()->GetBody()->GetUserData());
      if (!shield1)
      {
         shield1 = tmpShield;
      }
      else
      {
         shield2 = tmpShield;
      }
   }

   if (eB == CollisionEntity::shield)
   {
      Shield* tmpShield = BodyUserData::getParentObjectOfType<Shield*>(contact->GetFixtureB()->GetBody()->GetUserData());
      if (!shield1)
      {
         shield1 = tmpShield;
      }
      else
      {
         shield2 = tmpShield;
      }
   }

   if (eA == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::breakableObject)
   {
      breakable = BodyUserData::getParentObjectOfType<BreakableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (eA == CollisionEntity::explosiveObject)
   {
      explosive = BodyUserData::getParentObjectOfType<ExplosiveObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::explosiveObject)
   {
      explosive = BodyUserData::getParentObjectOfType<ExplosiveObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
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

   if (eA == CollisionEntity::steerableObject)
   {
      steerableObject = BodyUserData::getParentObjectOfType<SteerableObject*>(contact->GetFixtureA()->GetBody()->GetUserData());
   }

   if (eB == CollisionEntity::steerableObject)
   {
      steerableObject = BodyUserData::getParentObjectOfType<SteerableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if ((eA == CollisionEntity::flameParticle) || (eB == CollisionEntity::flameParticle))
   {
      flameParticle = true;      
   }

   if ((eA == CollisionEntity::blastParticle) || (eB == CollisionEntity::blastParticle))
   {
      blastParticle = true;
   }

   if (eB == CollisionEntity::steerableObject)
   {
      steerableObject = BodyUserData::getParentObjectOfType<SteerableObject*>(contact->GetFixtureB()->GetBody()->GetUserData());
   }

   if (bullet)
   {
      // Bullet hit anything, remove it
      bullet->bulletHit(contact);
   }
   
   if (breakable)
   {
      if (bullet)
      {
         breakable->damageCollision(contact, 1.0f);
      }
      else if (blastParticle)
      {
         breakable->damageCollision(contact, 0.5f);
      }
      else if (flameParticle)
      {
         breakable->damageCollision(contact, 0.02f);
      }
   }

   if (shield1)
   {
      if (bullet)
      {
         shield1->shieldHitByBullet(contact, 1.0f);
      }
      else if (blastParticle)
      {
         shield1->shieldHitByBullet(contact, 0.5f);
      }
      else
      {
         shield1->shieldHitImpulse(contact, impulse);
      }
   }

   if (shield2)
   {
      if (bullet)
      {
         shield2->shieldHitByBullet(contact, 1.0f);
      }
      else if (blastParticle)
      {
         shield2->shieldHitByBullet(contact, 0.5f);
      }
      else
      {
         shield2->shieldHitImpulse(contact, impulse);
      }
   }

   if (leapfrog)
   {
      // Normally. body hits are prevented by shield
      // but if it a hard hit body may get contact
      // (from certain angles). In this case, if the 
      // shield is still active, we do not damage
      // leapfrog 
      if (leapfrog->m_gameStatus->getShield() <= 0.0f)
      {
         if (bullet)
         {
            leapfrog->hitByBullet(contact, 1.0f);
         }
         else if (blastParticle)
         {
            leapfrog->hitByBullet(contact, 1.0f);
         }
         else
         {
            leapfrog->hitImpulse(contact, impulse);
         }
      }
   }

   // Hammer hit anything
   if (steerableObject)
   {
      if (bullet)
      {
         steerableObject->hitByBullet(contact, 1.0f);
      }
      else if (blastParticle)
      {
         steerableObject->hitByBullet(contact, 1.0f);
      }
      else
      {
         steerableObject->hitImpulse(contact, impulse);
      }
   }

   if (explosive)
   {
      if (bullet || blastParticle)
      { 
         explosive->triggerExplosion();
      }
   }
}


void FreeSpaceContactListener::BeginContact(b2Contact* contact)
{
}

void FreeSpaceContactListener::EndContact(b2Contact* contact)
{

}



