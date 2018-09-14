#include "steeringmanager.h"
#include "sceneactor.h"
#include "asteroid.h"

SteeringManager::SteeringManager(
   SteeringBody* hostBody,
   SceneActor* sceneActor)
 :
   m_hostBody(hostBody),
   m_sceneActor(sceneActor),
   m_wanderAngle(0.0f),
   m_steering(b2Vec2(0.0f, 0.0f))
{
}

void SteeringManager::update(void)
{
   // Always do asteroids collision avoidance
   m_steering += doAvoidCollision();

   b2Vec2 velocity = m_hostBody->GetLinearVelocity();

   if (m_steering.Length() > /*Max force*/ 1000.0f)
   {
      m_steering.Normalize();
      m_steering *= 1000.0f;
   }

   float invMass = 1 / m_hostBody->GetMass();

   m_steering = invMass * m_steering;
   
   velocity += m_steering;

   if (velocity.Length() > m_hostBody->GetMaxVelocity())
   {
      velocity.Normalize();
      velocity *= m_hostBody->GetMaxVelocity();
   }

   b2Vec2 velChange = velocity - m_hostBody->GetLinearVelocity();
   b2Vec2 impulse = m_hostBody->GetMass() * velChange;
   m_hostBody->m_body->ApplyLinearImpulse(impulse, m_hostBody->m_body->GetWorldCenter(), true);

}


void SteeringManager::seek(b2Vec2 target, float slowingRadius)
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
//	m_steering += doSeek(target->GetPosition());
   m_steering += doPursuit(target);
}

b2Vec2 SteeringManager::doSeek(b2Vec2 target, float slowingRadius)
{
   b2Vec2 force;
   float distance = 0.0f;
   
   b2Vec2 desired = target - m_hostBody->GetPosition();
   distance = desired.Length();
   desired.Normalize();
   
   if (distance < slowingRadius)
   {
      desired *= m_hostBody->GetMaxVelocity() * distance / slowingRadius;
   }
   else
   {
      desired *= m_hostBody->GetMaxVelocity();
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
   
   desired *= m_hostBody->GetMaxVelocity();

   force = desired - m_hostBody->GetLinearVelocity();
   
   return force;   
}

b2Vec2 SteeringManager::doWander()
{
   // Calculate the circle center
   b2Vec2 circleCenter;
   circleCenter = m_hostBody->GetLinearVelocity();
   circleCenter.Normalize();
   circleCenter *= m_hostBody->GetCircleDistance();

   //
   // Calculate the displacement force
   b2Vec2 displacement(1.0f, 0.0f);
   displacement *= m_hostBody->GetCircleRadius();

   // Randomly change the vector direction
   // by making it change its current angle
   float len = displacement.Length();
   displacement.x = cos(m_wanderAngle) * len;
   displacement.y = sin(m_wanderAngle) * len;

   //
   // Change wanderAngle just a bit, so it
   // won't have the same value in the
   // next game frame.

   m_wanderAngle += static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / m_hostBody->GetAngleChange())) - m_hostBody->GetAngleChange() / 2.0f;
   //
   // Finally calculate and return the wander force
   b2Vec2 wanderForce;
   wanderForce = circleCenter + displacement;
 
   return wanderForce + doStayInScene();
}

b2Vec2 SteeringManager::doEvade(b2Body* target)
{
   b2Vec2 distance = target->GetPosition() - m_hostBody->GetPosition();
   
   float updatesAhead = distance.Length() / m_hostBody->GetMaxVelocity();
   
   b2Vec2 futurePosition = updatesAhead * (target->GetPosition() + target->GetLinearVelocity());
   return doFlee(futurePosition);
}

b2Vec2 SteeringManager::doPursuit(b2Body* target)
{
   b2Vec2 force;
   b2Vec2 distance;
   
   distance = target->GetPosition() - m_hostBody->GetPosition();

   float updatesNeeded = distance.Length() / m_hostBody->GetMaxVelocity();

   b2Vec2 tv = target->GetLinearVelocity();
   tv *= updatesNeeded;

   b2Vec2 targetFuturePosition = target->GetPosition() + tv;

   return doSeek(targetFuturePosition);
}

b2Vec2 SteeringManager::doStayInScene(void)
{
   // Generate a force that maintains player in the scene
   // At the very edge, make ít the same speed as the player
   // but going in the direction of the boundary normal inwards.
   // But only use it close to the edge, i.e. scale it's strength
   // from 0 at 40 m from boundary and full at the edge

   float forceDist = 100.0f;
   b2Vec2 force = b2Vec2(0.0f, 0.0f);;
   b2Vec2 pos = m_hostBody->GetPosition();

   // Do top:

   if (pos.y < forceDist)
   {
      float edgeDist = pos.y - forceDist;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      force += b2Vec2(0.0f, velMag);
   }

   // Do bottom

   if (pos.y > m_sceneActor->m_sceneHeight - forceDist)
   {
      float edgeDist = m_sceneActor->m_sceneHeight - forceDist - pos.y;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      force += b2Vec2(0.0f, -velMag);
   }

   // Do left:

   if (pos.x < forceDist)
   {
      float edgeDist = pos.x - forceDist;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      force += b2Vec2(velMag, 0.0f);
   }

   // Do right

   if (pos.x > m_sceneActor->m_sceneWidth - forceDist)
   {
      float edgeDist = m_sceneActor->m_sceneWidth - forceDist - pos.x;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      force += b2Vec2(-velMag, 0.0f);
   }

   return force;
}

b2Vec2 SteeringManager::doAvoidCollision(void)
{
   b2Vec2 pos = m_hostBody->GetPosition();
   b2Vec2 vel = m_hostBody->GetLinearVelocity();

   float dynamicLength = vel.Length() / m_hostBody->GetMaxVelocity();

   vel.Normalize();

   b2Vec2 ahead = pos + dynamicLength * 80.0f * vel;
   b2Vec2 ahead2 = pos + dynamicLength * 80.0f * 0.5f * vel;

   m_debugAhead = ahead;
   m_debugAhead2 = ahead2;
   m_debugRadius = 2.0f;

   b2Body* mostThreatening = findMostThreatening(pos, ahead, ahead2);
   b2Vec2 avoidance = b2Vec2(0.0f, 0.0f);

   if (mostThreatening)
   {
      // Below should be avoidance = ahead - mostThreatening.GetPosition();
      //avoidance.x = ahead.x - mostThreatening->GetPosition().x;
      //avoidance.y = ahead.y - mostThreatening->GetPosition().y;
      avoidance = ahead - mostThreatening->GetPosition();

      avoidance.Normalize();

      avoidance *= 10.0f;

      logs::messageln("Avoidance.x: %f, Avoidance.y: %f", avoidance.x, avoidance.y);

   }

   return avoidance;
}

/*
private function collisionAvoidance() :Vector3D{
   ahead = ...; // calculate the ahead vector
ahead2 = ...; // calculate the ahead2 vector

var mostThreatening : Obstacle = findMostThreateningObstacle();
var avoidance : Vector3D = new Vector3D(0, 0, 0);

if (mostThreatening != null) {
   avoidance.x = ahead.x - mostThreatening.center.x;
   avoidance.y = ahead.y - mostThreatening.center.y;

   avoidance.normalize();
   avoidance.scaleBy(MAX_AVOID_FORCE);
}
else {
   avoidance.scaleBy(0); // nullify the avoidance force
}

return avoidance;
}

*/


b2Body* SteeringManager::findMostThreatening(b2Vec2 pos, b2Vec2 ahead, b2Vec2 ahead2)
{

   b2Body* mostThreatening = NULL;

   b2Body* body = m_hostBody->GetBodyList();
   while (body)
   {
      // So far, only avoid asteroids
      // Get collision entity
      CollisionEntity* ce = (CollisionEntity*)body->GetFixtureList()->GetUserData();
      if (ce->getEntityType() == CET_ASTEROID)
      {
         b2Vec2 asteroidPos = body->GetPosition();
         Asteroid* asteroid = (Asteroid*) body->GetUserData();
         float asteroidRadius = asteroid->getRadius();

         // Project asteroid velocity to vector from Asteroid to hammer
         // to see how fast the asteroid approaches
         b2Vec2 ast2pos = pos - asteroidPos;
         ast2pos.Normalize();
         float closingSpeed = b2Dot(ast2pos, body->GetLinearVelocity());

         // closingSpeed is the distance the asteroid will come closer
         // the next second. Add that to the radius
         // and also add the radius of the hammer 
         float collisionRadius = asteroidRadius + closingSpeed + 20.0f;

         float len = (asteroidPos - ahead).Length();
         float len2 = (asteroidPos - ahead2).Length();
         float len0 = (asteroidPos - pos).Length();

         if ((((asteroidPos - ahead).Length()) < collisionRadius) ||
            (((asteroidPos - ahead2).Length()) < collisionRadius) ||
            (((asteroidPos - pos).Length()) < collisionRadius))
         {
            if (mostThreatening == NULL)
            {
               mostThreatening = body;
               m_debugRadius = collisionRadius;
            }
            else
            {
               if ((pos - asteroidPos).Length() < ((pos - mostThreatening->GetPosition()).Length()))
               {
                  mostThreatening = body;
                  m_debugRadius = collisionRadius;
               }
            }
         }

      }

      b2Body* next = body->GetNext();
      body = next;
   }

   return mostThreatening;

}





b2Vec2 SteeringManager::getSteering()
{
   return m_steering;
}

void SteeringManager::reset(void)
{
   m_steering = b2Vec2(0.0f, 0.0f);
}
