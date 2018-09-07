#pragma once

#include "Box2D/Box2D.h"
#include "steeringinterface.h"

class SteeringBody
{
private:
   const b2Vec2 c_maxVelocity;
   const float c_wanderCircleDistance;
   const float c_wanderCircleRadius;
   const float c_wanderAngleChange;


public:
   b2Body * m_body;

   SteeringBody(
      b2Body* host,
      b2Vec2 maxVelocity,
      float wanderCircleDistance,
      float wanderCircleRadius,
      float wanderAngleChange
   );
		
   b2Vec2 GetPosition();
   b2Vec2 GetLinearVelocity();
   float GetMass();
   b2Vec2 GetMaxVelocity();

}
