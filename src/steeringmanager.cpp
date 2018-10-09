#include "steeringmanager.h"
#include "sceneactor.h"
#include "bodyuserdata.h"


SteeringManager::SteeringManager(
   b2Body* hostBody,
   SceneActor* sceneActor)
 :
   m_hostBody(hostBody),
   m_sceneActor(sceneActor),
   m_wanderAngle(0.0f),
   m_steering(b2Vec2(0.0f, 0.0f)),
   m_wanderHunterState(WanderHunterState::wanderState),
   m_fireTrigger(false)
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

   if (velocity.Length() > m_maxVelocity)
   {
      velocity.Normalize();
      velocity *= m_maxVelocity;
   }

   b2Vec2 velChange = velocity - m_hostBody->GetLinearVelocity();
   b2Vec2 impulse = m_hostBody->GetMass() * velChange;
   m_hostBody->ApplyLinearImpulse(impulse, m_hostBody->GetWorldCenter(), true);
}


void SteeringManager::seek(b2Vec2 target, float maxVelocity, float slowingRadius, float turnBooster)
{
   m_maxVelocity = maxVelocity;
	m_steering += doSeek(target, maxVelocity, slowingRadius, turnBooster);
}

void SteeringManager::flee(b2Vec2 target, float maxVelocity)
{
   m_maxVelocity = maxVelocity;
   m_steering += doFlee(target, maxVelocity);
}

void SteeringManager::wander(float maxVelocity)
{
   m_maxVelocity = maxVelocity;
   m_steering += doWander(maxVelocity);
}

void SteeringManager::evade(b2Body* target, float maxVelocity)
{
   m_maxVelocity = maxVelocity;
   m_steering += doFlee(target->GetPosition(), maxVelocity);
}

void SteeringManager::pursuit(b2Body* target, float maxVelocity)
{
   m_maxVelocity = maxVelocity;
   //	m_steering += doSeek(target->GetPosition());
   m_steering += doPursuit(target, maxVelocity);
}

void SteeringManager::wanderHunt(const UpdateState& us, b2Body* target, float maxVelocity)
{
   m_maxVelocity = maxVelocity;
   m_steering += doWanderHunt(us, target, maxVelocity);
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
   b2Vec2 force;
   float distance = 0.0f;
   
   b2Vec2 desired = target - m_hostBody->GetPosition();
   distance = desired.Length();

   b2Vec2 myVel = m_hostBody->GetLinearVelocity();
   
   // The turning booster force is a little extra hump
   // for turning when the velocity (and thus direction of travel)
   // is way off the target. The bigger the off-angle the bigger 
   // the boost. turnBoosterMag should be the angle between current 
   // travel and the desired direction. The angle is found using the
   // dot product.
   float dotProd = b2Dot(desired, myVel) / desired.Normalize() / myVel.Normalize();
   
   if (dotProd > 1.0f) dotProd = 1.0f;
   else if (dotProd < -1.0f) dotProd = -1.0f;
   
   float turnBoosterMagAngle = acos(dotProd);

   if (enableFire)
   {
      if (turnBoosterMagAngle < 5.0f / 180.0f * MATH_PI)
      {
         // We now point in the direction of the target, shot, if enabled
         m_fireTrigger = true;
      }
      else
      {
         m_fireTrigger = false;
      }
   }

   desired.Normalize();
   
   if (distance < slowingRadius)
   {
      desired *= maxVelocity * distance / slowingRadius;
   }
   else
   {
      desired *= maxVelocity;
   }

   // Lets put a ceiling in the turnBooster so it does not get
   // unnatural turning
   if (turnBoosterMagAngle > 1.57f)
   {
      turnBoosterMagAngle = 1.57f;
   }

   force = (desired - m_hostBody->GetLinearVelocity());
   force *= (turnBoosterMagAngle * turnBooster + 1.0f);

   return force;   
}

b2Vec2 SteeringManager::doFlee(b2Vec2 target, float maxVelocity)
{
   b2Vec2 force;
   float distance = 0.0f;
   
   b2Vec2 desired = m_hostBody->GetPosition() - target;
   distance = desired.Length();
   desired.Normalize();
   
   desired *= maxVelocity;

   force = desired - m_hostBody->GetLinearVelocity();
   
   return force;   
}

b2Vec2 SteeringManager::doWander(float maxVelocity)
{
   // Parameters yet to be trimmed
   float circleDistance = 10.0f;
   float circleRadius = 50.0f;
   float angleChange = 0.4f;

   // Calculate the circle center
   b2Vec2 circleCenter;
   circleCenter = m_hostBody->GetLinearVelocity();
   circleCenter.Normalize();
   circleCenter *= circleDistance;

   //
   // Calculate the displacement force
   b2Vec2 displacement(1.0f, 0.0f);
   displacement *= circleRadius;

   // Randomly change the vector direction
   // by making it change its current angle
   float len = displacement.Length();
   displacement.x = cos(m_wanderAngle) * len;
   displacement.y = sin(m_wanderAngle) * len;

   //
   // Change wanderAngle just a bit, so it
   // won't have the same value in the
   // next game frame.

   m_wanderAngle += static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / angleChange)) - angleChange / 2.0f;
   //
   // Finally calculate and return the wander force
   b2Vec2 wanderForce;
   wanderForce = circleCenter + displacement;
 
   return wanderForce + doStayInScene();
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
   b2Vec2 force;
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

   float dynamicLength = vel.Length() / m_maxVelocity;

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

      avoidance *= 500.0f;
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

            // logs::messageln("Pursuit");

            m_lastKnowTargetPos = target->GetPosition();
            m_maxVelocity = maxVelocity;
            return doPursuit(target, maxVelocity);
         }
      }

      // logs::messageln("Wander");

      m_maxVelocity = maxVelocity * 0.2f;
      return doWander(maxVelocity * 0.2f);
      
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

            // logs::messageln("Pursuit");

            m_lastKnowTargetPos = target->GetPosition();
            m_maxVelocity = maxVelocity;
            return doPursuit(target, maxVelocity);
         }
      }

      if ((m_hostBody->GetPosition() - m_lastKnowTargetPos).Length() < 20.0f)
      {
         // Go to wander
         m_wanderHunterState = WanderHunterState::wanderState;
         m_stateStartTime = us.time;

         // logs::messageln("Wander");

         m_maxVelocity = maxVelocity * 0.2f;
         return doWander(maxVelocity * 0.2f);
      }

      // logs::messageln("Seeking");

      m_maxVelocity = maxVelocity * 0.5f;
      return doSeek(m_lastKnowTargetPos, maxVelocity * 0.5f);

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

         // logs::messageln("Seeking");

         m_maxVelocity = maxVelocity * 0.5f;
         return doSeek(m_lastKnowTargetPos, maxVelocity * 0.5f);
      }
      else
      {
         // logs::messageln("Pursuit");

         m_lastKnowTargetPos = target->GetPosition();
         m_maxVelocity = maxVelocity;
         return doPursuit(target, maxVelocity);
      }

      break;
   }

   return b2Vec2(0.0f, 0.0f);
}




b2Vec2 SteeringManager::getSteering()
{
   return m_steering;
}

void SteeringManager::reset(void)
{
   m_steering = b2Vec2(0.0f, 0.0f);
}
