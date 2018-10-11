
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
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneActor, parentObject),
   m_gameResource(&gameResources),
   m_world(world),
   m_sceneActor(sceneActor),
   m_state(fix),
   m_initialAngle(0.0f),
   m_slowTurningAfterHit(false)
{
   initCompoundObjectParts(gameResources, sceneActor, parentObject, world, pos, root, string(""), groupIndex);

   readSteerableObjectNode(root);

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
   m_steeringManager->m_wanderAngle = m_initialAngle;
}

void SteerableObject::readSteerableObjectNode(pugi::xml_node root)
{
   xml_node node = root.child("behaviour");
   m_targetBody = m_sceneActor->getBody(node.attribute("targetBody").as_string());
   
   string st = node.attribute("steeringState").as_string();

   if (st == "wander") m_state = wander;
   else if (st == "seek") m_state = seek;
   else if (st == "pursuit") m_state = pursuit;
   else if (st == "flee") m_state = flee;
   else if (st == "wanderHunt") m_state = wanderHunt;
   else m_state = fix;

   m_initialAngle = node.attribute("initialAngle").as_float(0.0f);
   m_seekPoint.x = node.attribute("seekTargetX").as_float(0.0f);
   m_seekPoint.y = node.attribute("seekTargetY").as_float(0.0f);
}

void SteerableObject::doUpdate(const oxygine::UpdateState& us)
{
   switch (m_state)
   {
   case fix:
      break;
   case wanderHunt:
      m_steeringManager->wanderHunt(us, m_targetBody, 50.0f);
      m_steeringManager->update();
      break;
   case seek:
      m_steeringManager->seek(m_seekPoint, 25.0f);
      m_steeringManager->update();
      break;
   case wander:
      m_steeringManager->wander(10.0f);
      m_steeringManager->update();
      break;
   }

   // TODO: change flame states to more general speed 
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

 }



void SteerableObject::collisionBlast(b2Contact* contact, bool small)
// Sparks blastemitter, small is default, can also be big
{
   // Assume small blast
   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;

   if (!small)
   {
      emitterLifetime = 250;
      particleLifetime = 500;
      particleDistance = 60.0f;
      particleSize = 0.9f;
      blastIntensity = 300.0f;
   }

   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      spBlastEmitter blast = new BlastEmitter(
         m_sceneActor->getResources(),
         PhysDispConvert::convert(m.points[0], 1.0f),
         blastIntensity,                                     // Intensity, particles / sec
         emitterLifetime,                                    // Emitter Lifetime
         particleLifetime,                                   // Particle lifetime
         particleDistance,                                   // Particle distance
         particleSize);                                      // Particle spawn size
      blast->attachTo(m_sceneActor);
   }
}

void SteerableObject::evaluateDamage(void)
{
   if (m_gameStatus)
   {
      if (m_gameStatus->getDamage() > 100.0f)
      {
         addMeToDeathList();
      }
   }
}


void SteerableObject::hitImpulse(b2Contact* contact, const b2ContactImpulse* impulse)
{
   collisionBlast(contact, true);

   float normalImpulses = 0.0f;

   for (int i = 0; i < impulse->count; i++)
   {
      normalImpulses += impulse->normalImpulses[i];
   }

   if (m_gameStatus) m_gameStatus->deltaDamage(normalImpulses / 100.0f);

   evaluateDamage();
}

void SteerableObject::hitByBullet(b2Contact* contact, float bulletEqvDamage)
{
   collisionBlast(contact, true);

   if (m_gameStatus) m_gameStatus->deltaDamage(25.0f);

   evaluateDamage();
}
