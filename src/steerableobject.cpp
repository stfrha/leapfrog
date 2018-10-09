
#include "steerableobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

SteerableObject::SteerableObject(
   oxygine::Resources& gameResources,
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& root) :
   CompoundObject(sceneActor, parentObject),
   m_gameResource(&gameResources),
   m_world(world),
   m_sceneActor(sceneActor),
   m_damage(0),
   m_state(start),
   m_slowTurningAfterHit(false)
{
   Vector2 oSize = Vector2(24.0f, 12.6f);

   initCompoundObjectParts(gameResources, sceneActor, parentObject, world, pos, root, string(""));

   m_body = getBody("mainBody");

   if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
   {
      FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

      spaceActor->addBoundingBody(m_body);
   }


   // Add main engine particle system
   m_boosterFlame = static_cast<FlameEmitter*>(getSystem("boosterFlame"));

   m_boosterFlame->startEmitter();

   m_gun = static_cast<Gun*>(getSystem("gun"));

   attachTo(m_sceneActor);

   m_steeringManager = new SteeringManager(m_body, m_sceneActor);
   m_steeringManager->m_wanderAngle = MATH_PI;
}

void SteerableObject::hitByBullet(b2Contact* contact)
{
}

void SteerableObject::hitShield(b2Contact* contact)
{
}


void SteerableObject::hitByLepfrog(b2Contact* contact)
{
   // Take damage like two bullets
}



void SteerableObject::doUpdate(const oxygine::UpdateState& us)
{
//   m_steeringManager->wander();

//   m_steeringManager->seek(b2Vec2(100.0f, 450.0f));

   if (((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody != NULL)
   {
      // m_steeringManager->pursuit(((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody);
      m_steeringManager->wanderHunt(us, ((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody, 50.0f);
   }

   m_steeringManager->update();

   if (m_steeringManager->m_wanderHunterState == SteeringManager::WanderHunterState::wanderState)
   {
      m_boosterFlame->setFlameScale(0.2f);
   }
   else if (m_steeringManager->m_wanderHunterState == SteeringManager::WanderHunterState::seekState)
   {
      m_boosterFlame->setFlameScale(0.5f);
   }
   else
   {
      m_boosterFlame->setFlameScale(1.0f);
   }

   if (m_steeringManager->m_fireTrigger)
   {
      m_gun->startGun();
   }
   else
   {
      m_gun->stopGun();
   }
      
   //if (us.time >= m_stateStartTime + 5000)
   //{
   //   float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1000.0));
   //   float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 500.0));

   //   m_seekTarget = b2Vec2(x, y);


   //   m_stateStartTime = us.time;
   //}

   float bodyAngle = m_body->GetAngle();
   b2Vec2 toTarget = m_body->GetLinearVelocity();
   float desiredAngle = atan2f(-toTarget.x, toTarget.y) - MATH_PI / 2.0f;

   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   if (fabs(totalRotation) < 5.0f / 180.0f *  MATH_PI)
   {
      // Now a slow turn has found the desired angle for forward
      // movement, lets go back to fast rotations

      //if (m_slowTurningAfterHit)
      //{
      //   logs::messageln("Turning fast...");
      //}

      m_slowTurningAfterHit = false;
   }

   float desiredAngularVelocity = totalRotation * 60;

   if (m_slowTurningAfterHit)
   {
      float change = 2.5f / 180.0f * MATH_PI; //allow 1 degree rotation per time step
      desiredAngularVelocity = fmin(change, fmax(-change, desiredAngularVelocity));
   }
   float impulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor
   m_body->ApplyAngularImpulse(impulse, true);

   //m_aheadCircle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead, 1.0f));
   //m_aheadCircle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);
   //m_ahead2Circle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead2, 1.0f));
   //m_ahead2Circle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);


   //switch (m_state)
   //{
   //case start:
   //   m_stateStartTime = us.time;
   //   m_state = goRight;

   //   break;
   //case goRight:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going up
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(0.0f, -15.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goUp;
   //   }
   //   break;
   //case goUp:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going left
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(-15.0f, 0.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goLeft;
   //   }
   //   break;
   //case goLeft:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going down
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(0.0f, 15.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goDown;
   //   }
   //   break;
   //case goDown:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going right
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(15.0f, 0);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goRight;
   //   }
   //   break;
   //}
}

void SteerableObject::hitByAnything(b2Contact* contact)
{
   m_slowTurningAfterHit = true;
   // logs::messageln("Turning slow...");

}