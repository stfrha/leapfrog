#pragma once

#include "Box2D/Box2D.h"
#include "steeringinterface.h"

class SteeringBody
{
private:
   const float c_maxVelocity;
   const float c_wanderCircleDistance;
   const float c_wanderCircleRadius;
   const float c_wanderAngleChange;


public:
   b2Body * m_body;

   SteeringBody(
      b2Body* host,
      float maxVelocity,
      float wanderCircleDistance,
      float wanderCircleRadius,
      float wanderAngleChange
   );
		
   b2Vec2 GetPosition();
   b2Vec2 GetLinearVelocity();
   float GetMass();
   float GetMaxVelocity();
   float GetCircleDistance();
   float GetCircleRadius();
   float GetAngleChange();
   b2Body* GetBodyList();

};
