#pragma once

#include "Box2D/Box2D.h"
#include "steeringinterface.h"

class SteeringBody
{
private:

public:
   b2Body * m_body;

   SteeringBody(
      b2Body* host
   );
		
   b2Vec2 GetPosition();
   b2Vec2 GetLinearVelocity();
   float GetMass();
   b2Body* GetBodyList();

};
