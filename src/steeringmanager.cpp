#include "steeringmanager.h"

SteeringManager::SteeringManager(b2Body* host) :
	m_hostBody = host;
{
}
	
void SteeringManager::seek(b2Vect target, float slowingRadius = 0.0f)
{
	m_steering += doSeek(target, slowingRadius);
}

void SteeringManager::flee(b2Vect target)
{
	m_steering += doFlee(target);
}

void SteeringManager::wander()
{
	m_steering += doWander();
}

void SteeringManager::evade(b2Body* target)
{
	m_steering += doSeek(target);
}

void SteeringManager::pursuit(b2Body* target)
{
	m_steering += doSeek(target);
}

b2Vect SteeringManager::doSeek(b2Vect target, float slowingRadius = 0.0f)
{
   b2Vect force;
   float distance = 0.0f;
   
   b2Vect desired = target - host.getPosition();
   distance = desired.length();
   desired.normalize();
   
   if (distance < slowingRadius)
   {
      desired *= m_host.getMaxVelocity() * distance / slowingRadius;
   }
   else
   {
      desired *= target.getMaxVelocity();
   }

   force = desired - m_host.getVelocity;
   
   return force;   
}

b2Vect SteeringManager::doFlee(b2Vect target)
{
   b2Vect force;
   float distance = 0.0f;
   
   b2Vect desired = host.getPosition() - target;
   distance = desired.length();
   desired.normalize();
   
   desired *= target.getMaxVelocity();

   force = desired - m_host.getVelocity;
   
   return force;   
}

b2Vect SteeringManager::doWander()
{
   // Calculate the circle center
   b2Body circleCenter;
   circleCenter = m_host.getVelocity();
   circleCenter.normalize();
   circleCenter *= CIRCLE_DISTANCE;
   //
   // Calculate the displacement force
   b2Body displacement(0.0f, -1.0f);
   displacement =* CIRCLE_RADIUS;
   //
   // Randomly change the vector direction
   // by making it change its current angle
   float len = displacement.length;
   displacement.x = Math.cos(m_wanderAngle) * len;
   displacement.y = Math.sin(m_wanderAngle) * len;

   //
   // Change wanderAngle just a bit, so it
   // won't have the same value in the
   // next game frame.
   m_wanderAngle += Math.random() * ANGLE_CHANGE - ANGLE_CHANGE * 0.5f;
   //
   // Finally calculate and return the wander force
   b2Vect wanderForce;
   wanderForce = circleCenter + displacement;
 
   return wanderForce;
}

b2Vect SteeringManager::doEvade(b2Body* target)
{
   b2Vect distance = target.getPosition() - m_host.getPosition();
   
   float updatesAhead = distance.length() / m_target.getMaxVelocity();
   
   b2Vect futurePosition = target.getPosition + target.getVelocity * updatesAhead;
   return doFlee(futurePosition);
}

b2Vect SteeringManager::doPursuit(b2Body* target)
{
   b2Vect force;
   float distance = 0.0f;
   
   distance = target.getPosition() - m_host.getPosition();

   float updatesNeeded = distance.length / host.getMaxVelocity();

   b2Body tv = target.getVelocity();
   tv *= updatesNeeded;

   b2Vect targetFuturePosition = target.getPosition() + tv;

   return doSeek(targetFuturePosition);
}

