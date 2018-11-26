#include "steeringmanager.h"
#include "sceneactor.h"
#include "bodyuserdata.h"
#include "polygonvertices.h"

SteeringManager::SteeringManager(
   b2Body* hostBody,
   SceneActor* sceneActor)
 :
   m_hostBody(hostBody),
   m_sceneActor(sceneActor),
   m_wanderAngle(0.0f),
   m_wanderHunterState(WanderHunterState::wanderState),
   m_fireTrigger(false)
{
}


b2Vec2 SteeringManager::seek(b2Vec2 target, float maxVelocity, float slowingRadius, float turnBooster)
{
	return doSeek(target, maxVelocity, slowingRadius, turnBooster);
}

b2Vec2 SteeringManager::flee(b2Vec2 target, float maxVelocity)
{
   return  doFlee(target, maxVelocity);
}

b2Vec2 SteeringManager::wander(float maxVelocity)
{
   return doWander(maxVelocity);
}

b2Vec2 SteeringManager::evade(b2Body* target, float maxVelocity)
{
   return  doFlee(target->GetPosition(), maxVelocity);
}

b2Vec2 SteeringManager::pursuit(b2Body* target, float maxVelocity)
{
   //	m_steering += doSeek(target->GetPosition());
   return  doPursuit(target, maxVelocity);
}

b2Vec2 SteeringManager::wanderHunt(const UpdateState& us, b2Body* target, float maxVelocity)
{
   return doWanderHunt(us, target, maxVelocity);
}

b2Vec2 SteeringManager::avoidCollision(float maxVelocity)
{
   return doAvoidCollision(maxVelocity);
}

// doSeek, target is point to seek to, slowingRadius is the radius to target in which
// the host will slow down. turnBooster is the amount the turning force (and speed)
// is at off angles, 1 is no effect, 2 is enhanced, 4 is down right impossible to flee. 
b2Vec2 SteeringManager::doSeek(
   b2Vec2 target, 
   float maxVelocity,
   bool enableFire,
   float slowingRadius,
   float turnBooster)
{
   m_desiredVelocity = b2Vec2(0.0f, 0.0f);
   b2Vec2 velChange;
   float distance = 0.0f;
   
   m_desiredVelocity = target - m_hostBody->GetPosition();
   distance = m_desiredVelocity.Length();

   //// We want the turnBoosterMagAngle 
   //b2Vec2 myVel = m_hostBody->GetLinearVelocity();
   
   //// The turning booster velChange is a little extra hump
   //// for turning when the velocity (and thus direction of travel)
   //// is way off the target. The bigger the off-angle the bigger 
   //// the boost. turnBoosterMag should be the angle between current 
   //// travel and the desired direction. The angle is found using the
   //// dot product.
   //float dotProd = b2Dot(desired, myVel) / desired.Normalize() / myVel.Normalize();
   //
   //if (dotProd > 1.0f) dotProd = 1.0f;
   //else if (dotProd < -1.0f) dotProd = -1.0f;
   //
   //float turnBoosterMagAngle = acos(dotProd);

   if (enableFire)
   {
      //if (turnBoosterMagAngle < 5.0f / 180.0f * MATH_PI)
      //{
      //   // We now point in the direction of the target, shot, if enabled
      //   m_fireTrigger = true;
      //}
      //else
      //{
      //   m_fireTrigger = false;
      //}
   }

   m_desiredVelocity.Normalize();
   
   if (distance < slowingRadius)
   {
      m_desiredVelocity *= maxVelocity * distance / slowingRadius;
   }
   else
   {
      m_desiredVelocity *= maxVelocity;
   }

   //// Lets put a ceiling in the turnBooster so it does not get
   //// unnatural turning
   //if (turnBoosterMagAngle > 1.57f)
   //{
   //   turnBoosterMagAngle = 1.57f;
   //}



   // Sometimes, the booster push makes the object go ahead of
   // the maxVelocity which make the object try to turn to break
   // making it dance forward. Lets try to inhibit this behaviour.
   // We can not remove all velocity changes that are in the opposite
   // direction, it is needed to abruptly be able to change direction.
   // But if the desired velocity and the body velocity is, roughly,
   // in the same direction and the body speed is higher than the 
   // desired, we can set velChange to 0 and allow the linear
   // damping to slow the craft down gradually. 
   
   velChange = (m_desiredVelocity - m_hostBody->GetLinearVelocity());


//   velChange *= (turnBoosterMagAngle * turnBooster + 1.0f);

   //logs::messageln("speed: %f, velocityChange.X: %f",
   //   m_hostBody->GetLinearVelocity().Length(),
   //   velChange.x);



   return velChange;   
}

b2Vec2 SteeringManager::doFlee(b2Vec2 target, float maxVelocity)
{
   float distance = 0.0f;
   
   m_desiredVelocity = m_hostBody->GetPosition() - target;
   distance = m_desiredVelocity.Length();
   m_desiredVelocity.Normalize();
   
   m_desiredVelocity *= maxVelocity;
   
   b2Vec2 velChange;
   velChange = (m_desiredVelocity - m_hostBody->GetLinearVelocity());

   return velChange;
}

b2Vec2 SteeringManager::doWander(float maxVelocity)
{
   // Parameters yet to be trimmed
   float circleDistance = 50.0f;
   float circleRadius = 1.0f;
   float angleChange = 0.4f;

   // Calculate the circle center
   b2Vec2 circleCenter;
   circleCenter = m_hostBody->GetLinearVelocity();
   circleCenter.Normalize();
   circleCenter *= circleDistance;

   //
   // Calculate the displacement force
   // Randomly change the vector direction
   // by making it change its current angle
   b2Vec2 displacement(0.0f, 0.0f);

   displacement.x = cos(m_wanderAngle) * circleRadius;
   displacement.y = sin(m_wanderAngle) * circleRadius;

   //
   // Change wanderAngle just a bit, so it
   // won't have the same value in the
   // next game frame.

   float degOldWanderAngle = m_wanderAngle / MATH_PI * 180.0f;
   m_wanderAngle += static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / angleChange)) - angleChange / 2.0f;
   float degNewWanderAngle = m_wanderAngle / MATH_PI * 180.0f;
   float diff = degNewWanderAngle - degOldWanderAngle;



   //
   // Finally calculate and return the wander force
   m_desiredVelocity = circleCenter + displacement;

   // Scale down so that the desired velocity is not 
   // faster than the max velocity
   if (m_desiredVelocity.Length() > maxVelocity)
   {
      m_desiredVelocity.Normalize();
      m_desiredVelocity *= maxVelocity;
   }

   m_desiredVelocity += doStayInScene();
 
   b2Vec2 velChange;
   velChange = (m_desiredVelocity - m_hostBody->GetLinearVelocity());

   return velChange;
}

b2Vec2 SteeringManager::doEvade(b2Body* target, float maxVelocity)
{
   b2Vec2 distance = target->GetPosition() - m_hostBody->GetPosition();
   
   float updatesAhead = distance.Length() / maxVelocity;
   
   b2Vec2 futurePosition = updatesAhead * (target->GetPosition() + target->GetLinearVelocity());
   return doFlee(futurePosition, maxVelocity);
}

b2Vec2 SteeringManager::doPursuit(b2Body* target, float maxVelocity)
{
   b2Vec2 distance;
   
   distance = target->GetPosition() - m_hostBody->GetPosition();

   float updatesNeeded = distance.Length() / maxVelocity;

   b2Vec2 tv = target->GetLinearVelocity();
   tv *= updatesNeeded;

   b2Vec2 targetFuturePosition = target->GetPosition() + tv;

   return doSeek(targetFuturePosition, maxVelocity, true, 0.0f, 2.0f);
}

b2Vec2 SteeringManager::doStayInScene(void)
{
   // Generate a force that maintains player in the scene
   // At the very edge, make ít the same speed as the player
   // but going in the direction of the boundary normal inwards.
   // But only use it close to the edge, i.e. scale it's strength
   // from 0 at 40 m from boundary and full at the edge

   float forceDist = 100.0f;
   b2Vec2 velocity = b2Vec2(0.0f, 0.0f);;
   b2Vec2 pos = m_hostBody->GetPosition();

   // Do top:

   if (pos.y < forceDist)
   {
      float edgeDist = pos.y - forceDist;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      velocity += b2Vec2(0.0f, velMag);
   }

   // Do bottom

   if (pos.y > m_sceneActor->m_sceneHeight - forceDist)
   {
      float edgeDist = m_sceneActor->m_sceneHeight - forceDist - pos.y;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      velocity += b2Vec2(0.0f, -velMag);
   }

   // Do left:

   if (pos.x < forceDist)
   {
      float edgeDist = pos.x - forceDist;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      velocity += b2Vec2(velMag, 0.0f);
   }

   // Do right

   if (pos.x > m_sceneActor->m_sceneWidth - forceDist)
   {
      float edgeDist = m_sceneActor->m_sceneWidth - forceDist - pos.x;

      b2Vec2 vel = m_hostBody->GetLinearVelocity();
      float velMag = vel.Length();

      velMag = velMag - velMag * edgeDist / forceDist;

      velocity += b2Vec2(-velMag, 0.0f);
   }

   return velocity;
}

b2Vec2 SteeringManager::doAvoidCollision(float maxVelocity)
{
//   return doAvoidCollision1(maxVelocity);
   return doAvoidCollision2(maxVelocity);
}

b2Vec2 SteeringManager::doAvoidCollision2(float maxVelocity)
{
   // If we stand still, like at the beginning, there is 
   // no need to avoid obstacles, we simply return
   // a null-vector.
   if (m_hostBody->GetLinearVelocity().Length() < 0.0001)
   {
      return b2Vec2(0.0f, 0.0f);
   }

   b2Vec2 pos = m_hostBody->GetPosition();


   // Now find the angle of the velocity, we need it to all 
   // global-to-local transformations. 
   float angle = atan2(m_desiredVelocity.y, m_desiredVelocity.x);
   float degAngle = angle / MATH_PI * 180.0f;

   float cosAngle = cos(angle);
   float sinAngle = sin(angle);

   float mostThreatViolation = 0.0;
   float mostThreatDistance = FLT_MAX;
   float mostThreatDirection = 0.0f;

   // Assess to all obstacles
   b2Body* body = m_hostBody->GetWorld()->GetBodyList();
   while (body)
   {
      // So far, only avoid asteroids
      // Get collision entity
      if (BodyUserData::getCollisionType(body->GetFixtureList()->GetUserData()) == CollisionEntity::breakableObject)
      {
         b2Vec2 asteroidPos = body->GetPosition();

         // Ignore all that are beyond the avoidance distance
         if ((asteroidPos - pos).Length() < 200.0f)
         {
            //Asteroid* asteroid = (Asteroid*) body->GetUserData();
            float asteroidRadius = body->GetFixtureList()->GetShape()->m_radius;

            b2Vec2 localPos = PolygonVertices::globalToLocalConversion(pos, cosAngle, sinAngle, asteroidPos);
            //b2Vec2 localPos = m_hostBody->GetLocalPoint(asteroidPos);

            // Ignore all that are behind the object
            if (localPos.x > 0.0f)
            {
               // Check if the obstacle is within the path corridor
               float violation = asteroidRadius - fabs(localPos.y) + /* hammer radius*/ 30.0f;

               if (violation > 0.0f)
               {
                  if (localPos.x < mostThreatDistance)
                  {
                     mostThreatDistance = localPos.x;

                     if (localPos.y > 0)
                     {
                        mostThreatViolation = -violation;
                     }
                     else
                     {
                        mostThreatViolation = violation;
                     }
                  }
               }
            }
         }
      }

      b2Body* next = body->GetNext();
      body = next;
   }

   if (mostThreatDistance > /*very high*/ 200000.0f)
   {
      return b2Vec2(0.0f, 0.0f);
   }

   //b2Vec2 avoidance= m_hostBody->GetWorldPoint(b2Vec2(0.0f, mostThreatViolation)) - pos;
   b2Vec2 avoidance = PolygonVertices::localToGlobalConversion(pos, cosAngle, sinAngle, b2Vec2(0.0f, mostThreatViolation)) - pos;

   return avoidance;
}

b2Vec2 SteeringManager::doAvoidCollision1(float maxVelocity)
{
   b2Vec2 pos = m_hostBody->GetPosition();
   b2Vec2 vel = m_hostBody->GetLinearVelocity();

   float dynamicLength = vel.Length() / maxVelocity;

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

      avoidance *= 50000.0f;
   }

   return avoidance;
}

b2Body* SteeringManager::findMostThreatening(b2Vec2 pos, b2Vec2 ahead, b2Vec2 ahead2)
{

   b2Body* mostThreatening = NULL;

   b2Body* body = m_hostBody->GetWorld()->GetBodyList();
   while (body)
   {
      // So far, only avoid asteroids
      // Get collision entity
      if (BodyUserData::getCollisionType(body->GetFixtureList()->GetUserData()) == CollisionEntity::breakableObject)
      {
         b2Vec2 asteroidPos = body->GetPosition();
         //Asteroid* asteroid = (Asteroid*) body->GetUserData();
         float asteroidRadius = body->GetFixtureList()->GetShape()->m_radius;

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


// For the Raycast we need a callback class. Lets define it here
class MyRaycastCallback : public b2RayCastCallback
{
public:
   std::vector<b2Fixture*> m_fixtureList;
   bool m_targetIsHiding;

   float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
      const b2Vec2& normal, float32 fraction)
   {
      CollisionEntity::CollisionEntityTypeEnum ce = BodyUserData::getCollisionType(fixture->GetUserData());
      m_targetIsHiding = false;

      if ((ce == CollisionEntity::breakableObject) && (fraction < 1.0f))
      {
         m_targetIsHiding = true;
         return 0.0f;
      }

      return 1.0f;
   }

};



b2Vec2 SteeringManager::doWanderHunt(const UpdateState& us, b2Body* target, float maxVelocity)
{
   MyRaycastCallback mrcc;
   float pursuitVel = maxVelocity;
   float seekVel = maxVelocity * 0.7f;
   float wanderVel = maxVelocity * 0.5f;

   switch (m_wanderHunterState)
   {
   case WanderHunterState::wanderState:
      // While in wander, we look for target twice a second
      // If found we go to pursuit
      // If not found, we do the actual wander

      if (us.time >= m_stateStartTime + 500)
      {
         m_stateStartTime = us.time;

         m_hostBody->GetWorld()->RayCast(&mrcc, m_hostBody->GetPosition(), target->GetPosition());

         if (!mrcc.m_targetIsHiding)
         {
            // Go to pursuit
            m_wanderHunterState = WanderHunterState::pursuitState;

            logs::messageln("Pursuit");

            m_lastKnowTargetPos = target->GetPosition();
            return doPursuit(target, pursuitVel);
         }
      }

      logs::messageln("Wander");

      return doWander(wanderVel);
      
      break;

   case WanderHunterState::seekState:
      // While in seek, we look for target twice a second
      // If found we go to pursuit
      // If not found, we check if we reached last known 
      // position of target. If so, we go to Wander.
      // If not we do the actual seek

      if (us.time >= m_stateStartTime + 500)
      {
         m_stateStartTime = us.time;

         m_hostBody->GetWorld()->RayCast(&mrcc, m_hostBody->GetPosition(), target->GetPosition());

         if (!mrcc.m_targetIsHiding)
         {
            // Go to pursuit
            m_wanderHunterState = WanderHunterState::pursuitState;

            logs::messageln("Pursuit");

            m_lastKnowTargetPos = target->GetPosition();
            return doPursuit(target, pursuitVel);
         }
      }

      if ((m_hostBody->GetPosition() - m_lastKnowTargetPos).Length() < 20.0f)
      {
         // Go to wander
         m_wanderHunterState = WanderHunterState::wanderState;
         m_stateStartTime = us.time;

         logs::messageln("Wander");

         return doWander(wanderVel);
      }

      logs::messageln("Seeking");

      return doSeek(m_lastKnowTargetPos, seekVel);

      break;

   case WanderHunterState::pursuitState:

      // While in pursuit, we look for target at each update
      // If lost we go to seek, otherwise we pursuit

      m_hostBody->GetWorld()->RayCast(&mrcc, m_hostBody->GetPosition(), target->GetPosition());

      if (mrcc.m_targetIsHiding)
      {
         // Go to seek
         m_wanderHunterState = WanderHunterState::seekState;

         m_stateStartTime = us.time;

         logs::messageln("Seeking");

         return doSeek(m_lastKnowTargetPos, seekVel);
      }
      else
      {
         logs::messageln("Pursuit");

         m_lastKnowTargetPos = target->GetPosition();
         return doPursuit(target, pursuitVel);
      }

      break;
   }

   return b2Vec2(0.0f, 0.0f);
}

