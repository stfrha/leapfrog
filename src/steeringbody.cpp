
#include "steeringbody.h"

SteeringBody::SteeringBody(
   b2Body* host,
   float maxVelocity,
   float wanderCircleDistance,
   float wanderCircleRadius,
   float wanderAngleChange
) :
   m_body(host),
   c_maxVelocity(maxVelocity),
   c_wanderCircleDistance(wanderCircleDistance),
   c_wanderCircleRadius(wanderCircleRadius),
   c_wanderAngleChange(wanderAngleChange)
{

}

b2Vec2 SteeringBody::GetPosition()
{
   return m_body->GetPosition();
}

b2Vec2 SteeringBody::GetLinearVelocity()
{
   return m_body->GetLinearVelocity();
}


float SteeringBody::GetMass()
{
   return m_body->GetMass();

}


b2Vec2 SteeringBody::GetMaxVelocity()
{
   return c_maxVelocity;
}


