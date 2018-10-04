#pragma once

#include "oxygine-framework.h"
#include "collisionentity.h"

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
   CollisionEntityTypeEnum m_collisionType;
   oxygine::Actor* m_actor;

   template<class T>
   static T getActor(void* userData)
   {
      return static_cast<T>(static_cast<BodyUserData*>(userData)->m_actor);
   }

   static CollisionEntityTypeEnum getCollisionType(void* userData)
   {
      return static_cast<BodyUserData*>(userData)->m_collisionType;
   }


   // This method returns the userData pointer cast as type T
   // if the m_actor member of the supplied userData is of a
   // type 
   template<class T>
   static T getParentObjectOfType(void* userData)
   {
      oxygine::Actor* act = static_cast<BodyUserData*>(userData)->m_actor;

      while (act)
      {
         if (typeid(act) == typeid(T))
         {
            return static_cast<T>(act);
         }
         else
         {
            act = act->getParent();
         }
      }

      return NULL;
   }


};

