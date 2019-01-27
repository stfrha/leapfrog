#pragma once

#include "Box2d/Box2D.h"

class ISteering
{
public:
   virtual b2Vec2 GetPosition() = 0;
   virtual b2Vec2 GetLinearVelocity() = 0;
   virtual float GetMass() = 0;

   virtual b2Vec2 GetMaxVelocity() = 0;

};