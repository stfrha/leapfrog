#pragma once

#include <typeinfo>
#include <typeindex>
#include "oxygine-framework.h"
#include "collisionentity.h"
#include "actoruserdata.h"

DECLARE_SMART(BodyUserData, spBodyUserData);


// This class is used to set as user data for a b2Body. 
// It points to the actor and holds the collision entity
// The same BodyUserData object is to be pointed to by
// both the b2Body and all the b2Fixtures of the body
// The return of both fixture->GetUserData() and body->GetUserData() 
// can be used to get the same data.

class BodyUserData
{
public:
   CollisionEntity::CollisionEntityTypeEnum m_collisionType;
   oxygine::Actor* m_actor;

   template<class T>
   static T getActor(void* userData)
   {
      return static_cast<T>(static_cast<BodyUserData*>(userData)->m_actor);
   }

   static CollisionEntity::CollisionEntityTypeEnum getCollisionType(void* userData)
   {
      return static_cast<BodyUserData*>(userData)->m_collisionType;
   }


   // This method returns the userData pointer cast as type T
   // if the m_actor member of the supplied userData is of a
   // type inherited by type T.
   // If not, it looks up the ActorUserData.m_parentCo to see
   // if it is, etc. Until the parent is null.

   template<class T>
   static T getParentObjectOfType(void* userData)
   {
      oxygine::Actor* act = static_cast<BodyUserData*>(userData)->m_actor;

      while (act)
      {
         if (dynamic_cast<T>(act))
         {
            return static_cast<T>(act);
         }
         else
         {
            // We get the parent CompoundObject of the shape
            oxygine::Actor* testAct = ActorUserData::getParentObject(act->getUserData());

            if (testAct)
            {
               act = testAct;
            }
            else
            {
               // But if it is NULL:
               // If this was not a shape that pointed to its CO-parent
               // via the ActorUserData, then it was a CompoundObeject.
               // We must continue up the ladder but for a CO, the parent is
               // pointed as it's m_parentCo memeber. 
               act = static_cast<CompoundObject*>(act)->getParentObject();
            }
         }
      }

      return NULL;
   }


};

