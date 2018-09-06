#include "steeringmanager.h"

SteeringManager::SteeringManager(
   b2Body* hostBody,
   float maxVelocity,
   float wanderCircleDistance,
   float wanderCircleRadius,
   float wanderAngleChange
) :
   m_hostBody(hostBody),
   c_maxVelocity(maxVelocity),
   c_wanderCircleDistance(wanderCircleDistance),
   c_wanderCircleRadius(wanderCircleRadius),
   c_wanderAngleChange(wanderAngleChange)
{
}

void SteeringManager::seek(b2Vec2 target, float slowingRadius = 0.0f)
{
	m_steering += doSeek(target, slowingRadius);
}

void SteeringManager::flee(b2Vec2 target)
{
	m_steering += doFlee(target);
}

void SteeringManager::wander()
{
	m_steering += doWander();
}

void SteeringManager::evade(b2Body* target)
{
	m_steering += doFlee(target->GetPosition());
}

void SteeringManager::pursuit(b2Body* target)
{
	m_steering += doSeek(target->GetPosition());
}

b2Vec2 SteeringManager::doSeek(b2Vec2 target, float slowingRadius = 0.0f)
{
   b2Vec2 force;
   float distance = 0.0f;
   
   b2Vec2 desired = target - m_hostBody->GetPosition();
   distance = desired.Length();
   desired.Normalize();
   
   if (distance < slowingRadius)
   {
      desired *= m_hostBody->getMaxVelocity() * distance / slowingRadius;
   }
   else
   {
      desired *= target->getMaxVelocity();
   }

   force = desired - m_hostBody->GetLinearVelocity();
   
   return force;   
}

b2Vec2 SteeringManager::doFlee(b2Vec2 target)
{
   b2Vec2 force;
   float distance = 0.0f;
   
   b2Vec2 desired = m_hostBody->GetPosition() - target;
   distance = desired.Length();
   desired.Normalize();
   
   desired *= target->getMaxVelocity();

   force = desired - m_hostBody->GetLinearVelocity;
   
   return force;   
}

b2Vec2 SteeringManager::doWander()
{
   // Calculate the circle center
   b2Vec2 circleCenter;
   circleCenter = m_hostBody->GetLinearVelocity();
   circleCenter.Normalize();
   circleCenter *= CIRCLE_DISTANCE;
   //
   // Calculate the displacement force
   b2Vec2 displacement(0.0f, -1.0f);
   displacement =* CIRCLE_RADIUS;
   //
   // Randomly change the vector direction
   // by making it change its current angle
   float len = displacement.Length();
   displacement.x = cos(m_wanderAngle) * len;
   displacement.y = sin(m_wanderAngle) * len;

   //
   // Change wanderAngle just a bit, so it
   // won't have the same value in the
   // next game frame.
   m_wanderAngle += Math.random() * ANGLE_CHANGE - ANGLE_CHANGE * 0.5f;
   //
   // Finally calculate and return the wander force
   b2Vec2 wanderForce;
   wanderForce = circleCenter + displacement;
 
   return wanderForce;
}

b2Vec2 SteeringManager::doEvade(b2Body* target)
{
   b2Vec2 distance = target->GetPosition() - m_hostBody->GetPosition();
   
   float updatesAhead = distance.Length() / target->getMaxVelocity();
   
   b2Vec2 futurePosition = (target->GetPosition + target->GetLinearVelocity()) * updatesAhead;
   return doFlee(futurePosition);
}

b2Vec2 SteeringManager::doPursuit(b2Body* target)
{
   b2Vec2 force;
   b2Vec2 distance;
   
   distance = target->GetPosition() - m_hostBody->GetPosition();

   float updatesNeeded = distance.Length() / m_hostBody->getMaxVelocity();

   b2Vec2 tv = target->GetLinearVelocity();
   tv *= updatesNeeded;

   b2Vec2 targetFuturePosition = target->GetPosition() + tv;

   return doSeek(targetFuturePosition);
}

