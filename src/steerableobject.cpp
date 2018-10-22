
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
   b2Vec2 steeringForce = b2Vec2(0.0f, 0.0f);

   switch (m_state)
   {
   case fix:
      break;
   case wanderHunt:
      steeringForce = m_steeringManager->wanderHunt(us, m_targetBody, 50.0f);
      break;
   case seek:
      steeringForce = m_steeringManager->seek(m_seekPoint, 25.0f);
      break;
   case wander:
      steeringForce = m_steeringManager->wander(10.0f);
      break;
   }

   steeringForce += m_steeringManager->avoidCollision();

   executeSteeringForce(steeringForce);

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


 }

void SteerableObject::executeSteeringForce(b2Vec2 steeringForce)
{
   // This method is used to convert the steering force
   // into actual forces on the object body. This can 
   // be according to different viehcle models.
   // Each model is called from here
   turnBoosterForce(steeringForce);
}

void SteerableObject::firstTryForces(b2Vec2 steeringForce)
{
   b2Vec2 velocity = m_body->GetLinearVelocity();
   
   if (steeringForce.Length() > /*Max force*/ 1000.0f)
   {
      steeringForce.Normalize();
      steeringForce *= 1000.0f;
   }
   
   float invMass = 1 / m_body->GetMass();
   
   steeringForce = invMass * steeringForce;
      
   velocity += steeringForce;
   
   if (velocity.Length() > 20.0f /*m_maxVelocity*/)
   {
      velocity.Normalize();
      velocity *= 20.0f /*m_maxVelocity*/;
   }
   
   b2Vec2 velChange = velocity - m_body->GetLinearVelocity();
   b2Vec2 impulse = m_body->GetMass() * velChange;
   m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);







   float bodyAngle = m_body->GetAngle();
   b2Vec2 toTarget = m_body->GetLinearVelocity();
   float desiredAngle = atan2f(-toTarget.x, toTarget.y) - MATH_PI / 2.0f;

   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   float desiredAngularVelocity = totalRotation * 60;
   float change = 1.0f / 180.0f * MATH_PI;
   desiredAngularVelocity = min(change, max(-change, desiredAngularVelocity));
   float angImpulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor
   m_body->ApplyAngularImpulse(angImpulse, true);

   //m_aheadCircle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead, 1.0f));
   //m_aheadCircle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);
   //m_ahead2Circle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead2, 1.0f));
   //m_ahead2Circle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);

}

void SteerableObject::turnBoosterForce(b2Vec2 steeringForce)
{
   // In this model, the booster can only supply linear push in its pointing direction 
   // which is quite real. The ship need to turn in the direction of the 
   // desired force and give booster burn in that direction. To be more efficient
   // the booster force is not only applied when the ship points exactly 
   // in the force direction. It projects the force on the normalised velocity 
   // vector and the result is the booster force. 
   // The magnitude of the booster force is truncated to the maximum force setting
   // of the veihcel model. 
   // The angular force is the force needed to turn the ship to the force direction 
   // in N steps. (N is TBD) taking into account the current angle and the 
   // angle inertia. The resulting angular force is truncated to the maximum 
   // angular force of the ship.
   
   
   // Find the boost force which is to be applied to the 
   // booster direction. It is the steering force projected
   // on the direction of the ship

   // If the ship is stationary, the velocity is zero then there is no directon
   // which cause undefined (and probably erratic) behaviour
   // Thus we can not use vectors for the angle calculations. We need angles
   float objectAngle = m_body->GetAngle();

   while (objectAngle > MATH_PI) objectAngle -= 2.0f * MATH_PI;
   while (objectAngle < -MATH_PI) objectAngle += 2.0f * MATH_PI;

   b2Vec2 objectDirection = b2Vec2(cos(objectAngle),-sin(objectAngle));

   float objectSpeed = m_body->GetLinearVelocity().Length();

   // Project steeringForce onto objectDirection
   float projectedForce = b2Dot(steeringForce, objectDirection);
   
   // In this model, booster can not be reversed, Ship must turn
   if (projectedForce < 0.0f)
   {
      projectedForce = 0.0f;
   }
   else if (projectedForce > /* maxBoosterForce */ 10000.0f)
   {
      projectedForce = /* maxBoosterForce */ 10000.0f;
   }


   b2Vec2 boostForce = projectedForce * objectDirection;


   // Find the angular force to turn the ship in the direction of
   b2Vec2 toTarget = steeringForce;
   float desiredAngle = atan2f(-toTarget.x, toTarget.y) - /* Is this the initiatial angle?*/MATH_PI / 2.0f;

//   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float nextAngle = objectAngle + m_body->GetAngularVelocity() / 3.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   float desiredAngularVelocity = totalRotation * 60;
   float change = 3.0f / 180.0f * MATH_PI;
   desiredAngularVelocity = min(change, max(-change, desiredAngularVelocity));
   float angImpulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor



   // Apply the forces
   m_body->ApplyLinearImpulseToCenter(boostForce, true);
   m_body->ApplyAngularImpulse(angImpulse, true);

   logs::messageln("objectAngle: %f, desiredAngle: %f, X: %f, Y: %f, ang: %f", 
      objectAngle / MATH_PI * 180.0f, 
      desiredAngle / MATH_PI * 180.0f,
      boostForce.x,
      boostForce.y, 
      angImpulse);



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
