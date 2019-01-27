#pragma once

#include "Box2d/Box2D.h"
#include "sceneactor.h"

class FreeSpaceContactListener : public b2ContactListener
{
public:
   void BeginContact(b2Contact* contact);
   void EndContact(b2Contact* contact);
   void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
   void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};
