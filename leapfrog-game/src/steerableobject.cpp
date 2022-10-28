
#include "steerableobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"
#include "polygonvertices.h"
#include "headdowndisplay.h"

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
//   m_state(fix),
   m_bodyToBoosterAngle(0.0f),
   m_maxBoosterForce(0.0f),
   m_maxSpeed(0.0f),
   m_maxRotateSpeed(0.0f),
   m_linearDamping(1.0f),
   m_boosterScale(0.0f),
   m_slowTurningAfterHit(false),
   m_aimState(noAim),
   m_shield(NULL),
   m_gun(NULL),
   m_boosterFlame(NULL),
   m_armClockReset(true)
{
   initCompoundObjectParts(gameResources, sceneActor, sceneActor, parentObject, world, pos, root, groupIndex);

   attachTo(m_sceneActor);

   // Register all properties:
   m_properties.push_back(ObjectProperty(this, NULL, 0, 4.0f)); // Mode, default to wander-hunt
   m_properties.push_back(ObjectProperty(this, NULL, 1, 0.0f, true)); // xPos
   m_properties.push_back(ObjectProperty(this, NULL, 2, 0.0f, true)); // yPos
   m_properties.push_back(ObjectProperty(this, NULL, 3, 0.0f, true)); // Seek xPos
   m_properties.push_back(ObjectProperty(this, NULL, 4, 0.0f, true)); // Seek yPos

   readSteerableObjectNode(root.child("behaviour").child("steerableObjectProperties"));

   m_body = getBody("mainBody");

   if (m_sceneActor->getSceneType() == SceneActor::SceneTypeEnum::deepSpace)
   {
      FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

      spaceActor->addBoundingBody(m_body);
   }

   // Add main engine particle system
   m_boosterFlame = static_cast<FlameEmitter*>(getSystem("boosterFlame"));

   m_boosterFlame->startEmitter();

   m_gun = static_cast<Gun*>(getSystem("gun"));

   m_shield = static_cast<Shield*>(getSystem("shield"));

   m_steeringManager = new SteeringManager(m_body);
   m_steeringManager->m_wanderAngle = m_bodyToBoosterAngle;

   m_body->SetLinearDamping(m_linearDamping);

   m_headDownDisplayItemId = g_HeadDownDisplay->addMeToMap(
      MapItem::MapItemTypeEnum::enemyMoving,
      getActor("mainBody"),
      MapItem::MapItemStateEnum::hollow);
}

void SteerableObject::readSteerableObjectNode(pugi::xml_node node)
{
   m_targetBodyStr = node.attribute("targetBody").as_string();
   
   string st = node.attribute("steeringState").as_string();

   if (st == "wander") m_properties[SteerableObjectPropertyEnum::state].setProperty(wander);
   else if (st == "seek") m_properties[SteerableObjectPropertyEnum::state].setProperty(seek);
   else if (st == "pursuit") m_properties[SteerableObjectPropertyEnum::state].setProperty(pursuit);
   else if (st == "flee") m_properties[SteerableObjectPropertyEnum::state].setProperty(flee);
   else if (st == "wanderHunt") m_properties[SteerableObjectPropertyEnum::state].setProperty(wanderHunt);
   else m_properties[SteerableObjectPropertyEnum::state].setProperty(fix);

   m_bodyToBoosterAngle = node.attribute("bodyToBoosterAngle").as_float(0.0f) * MATH_PI / 180.0f;
   m_seekPoint.x = node.attribute("seekTargetX").as_float(0.0f);
   m_seekPoint.y = node.attribute("seekTargetY").as_float(0.0f);

   m_maxBoosterForce = node.attribute("maxBoosterForce").as_float(0.0f);
   m_maxSpeed = node.attribute("maxSpeed").as_float(0.0f);
   m_maxRotateSpeed = node.attribute("degPerSecMaxRotateSpeed").as_float(0.0f) / 180.0f * MATH_PI;
   m_linearDamping = node.attribute("linearDamping").as_float(0.0f);
}

void SteerableObject::connectToForeignObjects(void)
{
   m_targetBody = m_sceneActor->getBody(m_targetBodyStr);

   if (m_targetBody == NULL)
   {
      logs::messageln("Steerable object did not find the target bode: %s", m_targetBodyStr.c_str());
   }

   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      (*it)->connectToForeignObjects();
   }
}


void SteerableObject::doUpdate(const oxygine::UpdateState& us)
{
   // Do a one-time clock synch
   if (m_armClockReset)
   {
      m_armClockReset = false;
      m_steeringManager->resetStateChangeClock(us);
   }

   if (!m_sceneActor->getIsInPause())
   {
      b2Vec2 pos = m_body->GetPosition();
      m_properties[SteerableObjectPropertyEnum::xPos].setProperty(pos.x);
      m_properties[SteerableObjectPropertyEnum::yPos].setProperty(pos.y);


      b2Vec2 steeringVelChange(0.0f, 0.0f);
      int state = (int)m_properties[SteerableObjectPropertyEnum::state].getProperty();
      switch (state)
      {
      case fix:
         break;
      case wanderHunt:
         steeringVelChange = m_steeringManager->wanderHunt(us, m_targetBody, m_maxSpeed);
         if (m_steeringManager->getWanderHunterStateChanged())
         {
            SteeringManager::WanderHunterState whState = m_steeringManager->getWanderHunterState();
            switch (whState)
            {
            case SteeringManager::WanderHunterState::wanderState:
               g_HeadDownDisplay->setState(m_headDownDisplayItemId, MapItem::MapItemStateEnum::hollow);
               break;
            case SteeringManager::WanderHunterState::seekState:
               g_HeadDownDisplay->setState(m_headDownDisplayItemId, MapItem::MapItemStateEnum::flashingSlow);
               break;
            case SteeringManager::WanderHunterState::pursuitState:
               g_HeadDownDisplay->setState(m_headDownDisplayItemId, MapItem::MapItemStateEnum::flashingFast);
               break;
            }
         }
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

      b2Vec2 avoidanceVelChange = m_steeringManager->avoidCollision(m_maxSpeed);

      steeringVelChange += avoidanceVelChange;

      executeSteeringForce(steeringVelChange, us, (avoidanceVelChange.Length() > 0.00001f));

      m_boosterFlame->setFlameScale(m_boosterScale);
   }

 }

void SteerableObject::executeSteeringForce(b2Vec2 steeringVelChange, const UpdateState& us, bool isAvoiding)
{
   // This method is used to convert the steering force
   // into actual forces on the object body. This can 
   // be according to different vehicle models.
   // Each model is called from here
//   firstTryForces(steeringVelChange);
//   turnBoosterForce(steeringVelChange);

   // This also handle periodically turning
   // to aim gun at target. We need a state machine
   // that swaps between direction of velocity
   // and direction of target. 
   // There should also be some extra requirements on
   // the turning. 
   // 1. Only aim if there are no obstacles!
   // 2. Only aim in pursuit mode
   // 3. Only aim when in gun range (400 m)

   switch (m_aimState)
   {
   case noAim:
      // We go to aiming every 2 s 
      if (us.time >= m_aimStateStartTime + 2000)
      {
         logs::messageln("Aim");

         m_aimState = aim;
         m_aimStateStartTime = us.time;
      }
      break;
   case aim:
      // We allow 0.5 s to aim
      if (us.time >= m_aimStateStartTime + 500)
      {
         logs::messageln("Shoot");

         m_aimState = shoot;
         m_aimStateStartTime = us.time;
         m_gun->startGun();
      }

      if (isAvoiding ||
         (m_steeringManager->peekWanderHunterState() != SteeringManager::WanderHunterState::pursuitState) ||
         ((m_targetBody->GetPosition() - m_body->GetPosition()).Length() > /* bullet range*/ 400.0f))
      {
         m_aimState = noAim;
         m_aimStateStartTime = us.time;
         m_gun->stopGun();
      }
      break;
   case shoot:
      // 0.5 s shot burst
      if (us.time >= m_aimStateStartTime + 500)
      {
         logs::messageln("Stop shoot");

         m_aimState = noAim;
         m_aimStateStartTime = us.time;
         m_gun->stopGun();
      }

      if (isAvoiding ||
         (m_steeringManager->peekWanderHunterState() != SteeringManager::WanderHunterState::pursuitState) ||
         ((m_targetBody->GetPosition() - m_body->GetPosition()).Length() > /* bullet range*/ 400.0f))
      {
         m_aimState = noAim;
         m_aimStateStartTime = us.time;
         m_gun->stopGun();
      }
      break;
   }

   directiveForce(steeringVelChange, m_aimState != noAim);
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

void SteerableObject::directiveForce(b2Vec2 steeringVelChange, bool aim)
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

   // If the gun is being aimed, the target is the target body position
   // but if gun is not being aimed, the target is the steering 
   // velocity change as determined by the previous steering
   // behaviours
   b2Vec2 toTarget(0.0f, 0.0f);

   if (aim)
   {
      // toTarget = m_targetBody->GetPosition() - m_body->GetPosition();
      
      b2Vec2 distance;

      distance = m_targetBody->GetPosition() - m_body->GetPosition();

      float updatesNeeded = distance.Length() / /* bullet speed?? */ 200.0f ;

      b2Vec2 tv = m_targetBody->GetLinearVelocity();
      tv *= updatesNeeded;

      toTarget = m_targetBody->GetPosition() + tv - m_body->GetPosition();
   }
   else
   {
      toTarget = steeringVelChange;
   }

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
   if (m_resources)
   {
      if (m_resources->getDamage() > 100.0f)
      {
         addMeToDeathList();
         g_HeadDownDisplay->removeMeFromMap(getActor("mainBody"));

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

   if (m_resources) m_resources->deltaDamage(normalImpulses / 100.0f);

   evaluateDamage();
}

void SteerableObject::hitByBullet(b2Contact* contact, float bulletEqvDamage)
{
   collisionBlast(contact, true);

   if (m_resources) m_resources->deltaDamage(25.0f);

   evaluateDamage();
}

// The steerable object does not need any external connection 
// of the resources. Therefore it is created, locally here
void SteerableObject::initObjectResources(Actor* statusEventOriginator, spObjectResources resources)
{
   m_resources = new ObjectResources();

   ObjectProperty* ammo = new ObjectProperty(this, NULL, 5, 0.0f);
   ObjectProperty* shield = new ObjectProperty(this, NULL, 6, 0.0f);
   ObjectProperty* fuel = new ObjectProperty(this, NULL, 7, 0.0f);
   ObjectProperty* credits = new ObjectProperty(this, NULL, 8, 0.0f);
   ObjectProperty* damage = new ObjectProperty(this, NULL, 9, 0.0f);

   m_resources->initObjectResources(statusEventOriginator, ammo, shield, fuel, credits, damage);

   m_properties.push_back(*ammo);
   m_properties.push_back(*shield);
   m_properties.push_back(*fuel);
   m_properties.push_back(*credits);
   m_properties.push_back(*damage);

   m_resources->registerShieldObject(m_shield.get());
}
