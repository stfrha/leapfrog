
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
   m_bodyToBoosterAngle(0.0f),
   m_maxBoosterForce(0.0f),
   m_maxSpeed(0.0f),
   m_maxRotateSpeed(0.0f),
   m_linearDamping(1.0f),
   m_boosterScale(0.0f),
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
   m_steeringManager->m_wanderAngle = m_bodyToBoosterAngle;

   m_body->SetLinearDamping(m_linearDamping);
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

   m_bodyToBoosterAngle = node.attribute("bodyToBoosterAngle").as_float(0.0f);
   m_seekPoint.x = node.attribute("seekTargetX").as_float(0.0f);
   m_seekPoint.y = node.attribute("seekTargetY").as_float(0.0f);

   m_maxBoosterForce = node.attribute("maxBoosterForce").as_float(0.0f);
   m_maxSpeed = node.attribute("maxSpeed").as_float(0.0f);
   m_maxRotateSpeed = node.attribute("degPerSecMaxRotateSpeed").as_float(0.0f) / 180.0f * MATH_PI;
   m_linearDamping = node.attribute("linearDamping").as_float(0.0f);
}

void SteerableObject::doUpdate(const oxygine::UpdateState& us)
{
   b2Vec2 steeringVelChange = b2Vec2(0.0f, 0.0f);

   switch (m_state)
   {
   case fix:
      break;
   case wanderHunt:
      steeringVelChange = m_steeringManager->wanderHunt(us, m_targetBody, m_maxSpeed);
      break;
   case seek:
      steeringVelChange = m_steeringManager->seek(m_seekPoint, m_maxSpeed);
      break;
   case pursuit:
      steeringVelChange = m_steeringManager->pursuit(m_targetBody, m_maxSpeed);
      break;
   case wander:
      steeringVelChange = m_steeringManager->wander(m_maxSpeed);
      break;
   }

   steeringVelChange += m_steeringManager->avoidCollision(m_maxSpeed);

   executeSteeringForce(steeringVelChange);

   m_boosterFlame->setFlameScale(m_boosterScale);

   if (m_steeringManager->evaluateGunFire(m_targetBody))
   {
      m_gun->startGun();
   }
   else
   {
      m_gun->stopGun();
   }
 }

void SteerableObject::executeSteeringForce(b2Vec2 steeringVelChange)
{
   // This method is used to convert the steering force
   // into actual forces on the object body. This can 
   // be according to different viehcle models.
   // Each model is called from here
//   firstTryForces(steeringVelChange);
//   turnBoosterForce(steeringVelChange);
   directiveForce(steeringVelChange);
}

void SteerableObject::firstTryForces(b2Vec2 steeringVelChange)
{
   b2Vec2 velocity = m_body->GetLinearVelocity();
   
   if (steeringVelChange.Length() > /*Max force*/ 1000.0f)
   {
      steeringVelChange.Normalize();
      steeringVelChange *= 1000.0f;
   }
   
   float invMass = 1 / m_body->GetMass();
   
   steeringVelChange = invMass * steeringVelChange;
      
   velocity += steeringVelChange;
   
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

void SteerableObject::turnBoosterForce(b2Vec2 steeringVelChange)
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

   // thrustAngle is the direction of the booster push which may not
   // necessary be the body direction. 
   float bodyAngle = m_body->GetAngle();
   float thrustAngle = bodyAngle - m_bodyToBoosterAngle;
   float degAngle = thrustAngle / MATH_PI * 180.0f;

   while (thrustAngle > MATH_PI) thrustAngle -= 2.0f * MATH_PI;
   while (thrustAngle < -MATH_PI) thrustAngle += 2.0f * MATH_PI;

   b2Vec2 objectDirection = b2Vec2(cos(thrustAngle),sin(thrustAngle));

   float objectSpeed = m_body->GetLinearVelocity().Length();

   float steeringSpeedChange = steeringVelChange.Length();

   // Get angle between desired velocity change and the body direction
   float boostAngle = acos(b2Dot(steeringVelChange, objectDirection) / steeringSpeedChange);
   float degBoosAngle = boostAngle / MATH_PI * 180.0f;

   b2Vec2 boostForce = b2Vec2(0.0f, 0.0f);

   float boostCone = 22.0f / 180.0f * MATH_PI;

   // Only boost if we are pointing in, roughly, the right direction
   if (boostAngle < boostCone)
   {
      // Scale the force so that we get max when we point directly 
      // in the velocity change and zero at the boundary angle
      float projectedForce = m_body->GetMass() * (1.0f - boostAngle / boostCone) * steeringSpeedChange;
    
      // In this model, booster can not be reversed, Ship must turn
      if (projectedForce < 0.0f)
      {
         projectedForce = 0.0f;
      }
      else if (projectedForce > m_maxBoosterForce)
      {
         projectedForce = m_maxBoosterForce;
      }

      m_boosterScale = projectedForce / m_maxBoosterForce;

      boostForce = projectedForce * objectDirection;
   }
   

   // Find the angular force to turn the ship in the direction 
   // to get the desired velocity change
   b2Vec2 toTarget = steeringVelChange;
   float desiredAngle = atan2f(toTarget.y, toTarget.x);

//   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float nextAngle = thrustAngle + m_body->GetAngularVelocity() / 3.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   float desiredAngularVelocity = totalRotation * 60;
   float change = m_maxRotateSpeed;
   desiredAngularVelocity = min(change, max(-change, desiredAngularVelocity));
   float angImpulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor



   // Apply the forces
   m_body->ApplyLinearImpulseToCenter(boostForce, true);
   m_body->ApplyAngularImpulse(angImpulse, true);


   //logs::messageln("thrustAngle: %f, desiredAngle: %f, X: %f, Y: %f, ang: %f", 
   //   thrustAngle / MATH_PI * 180.0f, 
   //   desiredAngle / MATH_PI * 180.0f,
   //   //boostForce.x,
   //   //boostForce.y, 
   //   //steeringVelChange.x,
   //   //steeringVelChange.y,
   //   m_body->GetPosition().x,
   //   m_body->GetPosition().y,
   //   angImpulse);



}

void SteerableObject::directiveForce(b2Vec2 steeringVelChange)
{
   // In this model, the booster applies linear force in the direction of
   // the steering velocity change. It then use torque force to try to angle the 
   // object to point in the same direction. The turn will be a little late but
   // maybe not in a totally bad way. 
   // The angular force is the force needed to turn the ship to the force direction 
   // in N steps. (N is TBD) taking into account the current angle and the 
   // angle inertia. The resulting angular force is truncated to the maximum 
   // angular force of the ship.

   b2Vec2 impulse = m_body->GetMass() * steeringVelChange;

   float boostMag = impulse.Length();

   if (boostMag > m_maxBoosterForce)
   {
      impulse.Normalize();
      impulse *= m_maxBoosterForce;
      m_boosterScale = 1.0f;
   }
   else
   {
      m_boosterScale = boostMag / m_maxBoosterForce;
   }




   // Find the angular force to turn the ship in the direction 
   // to get the desired velocity change
   b2Vec2 toTarget = steeringVelChange;
   float desiredAngle = atan2f(toTarget.y, toTarget.x);

   float bodyAngle = m_body->GetAngle();
   float thrustAngle = bodyAngle - m_bodyToBoosterAngle;

   //   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float nextAngle = thrustAngle + m_body->GetAngularVelocity() / 3.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   float desiredAngularVelocity = totalRotation * 60;
   float change = m_maxRotateSpeed;
   desiredAngularVelocity = min(change, max(-change, desiredAngularVelocity));
   float angImpulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor

   // Apply the forces
   m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);
   m_body->ApplyAngularImpulse(angImpulse, true);

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
