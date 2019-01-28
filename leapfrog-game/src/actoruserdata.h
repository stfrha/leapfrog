#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"

DECLARE_SMART(BodyUserData, spBodyUserData);


// This class is used to set as user data for a b2Body. 
// It points to the actor and holds the collision entity
// The same BodyUserData object is to be pointed to by
// both the b2Body and all the b2Fixtures of the body
// The return of both fixture->GetUserData() and body->GetUserData() 
// can be used to get the same data.

class ActorUserData
{
public:
   b2Body* m_body;
   CompoundObject* m_parentCo;

   static b2Body* getBody(void* userData)
   {
      return static_cast<ActorUserData*>(userData)->m_body;
   }

   static b2Body* getBody(const void* userData)
   {
      const ActorUserData* ud = static_cast<const ActorUserData*>(userData);

      if (ud)
      {
         return static_cast<const ActorUserData*>(userData)->m_body;
      }

      return NULL;
   }

   static CompoundObject* getParentObject(const void* userData)
   {
      if (userData)
      {
         const ActorUserData* a = static_cast<const ActorUserData*>(userData);

         return static_cast<const ActorUserData*>(userData)->m_parentCo;
      }

      return NULL;
   }

};

