#include "dockingstation.h"

#include "mainactor.h"
#include "sceneactor.h"
#include "leapfrog.h"
#include "shield.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "headdowndisplay.h"

#include <cmath>

using namespace oxygine;
using namespace std;

namespace
{
   const float c_pi = 3.14159265f;

   float wrapAngle(float a)
   {
      return remainderf(a, 2.0f * c_pi);
   }

   float clampf(float v, float lo, float hi)
   {
      return (v < lo) ? lo : ((v > hi) ? hi : v);
   }

   const oxygine::timeMS c_dockZoomMs = 3000;
   const oxygine::timeMS c_undockZoomMs = 2000;
}

DockingStation::DockingStation(
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_world(world),
   m_base(NULL),
   m_arm(NULL),
   m_clamp(NULL),
   m_anchor(NULL),
   m_keeper(NULL),
   m_shoulder(NULL),
   m_dockJoint(NULL),
   m_state(ready),
   m_stateTime(0.0f),
   m_active(true),
   m_boosterReleased(false),
   m_dockedSent(false),
   m_announceDocked(false),
   m_refusalShown(false),
   m_clampPoint(0.0f, 0.0f),
   m_nosePoint(0.0f, -5.25f),
   m_dockAngle(0.0f),
   m_armTarget(0.0f),
   m_armMoving(false)
{
   initCompoundObjectParts(sceneParent, sceneParent, parentObject, world, pos, root, groupIndex);

   pugi::xml_node behaviour = root.child("behaviour");
   pugi::xml_node props = behaviour.child("dockingStationProperties");

   m_active = behaviour.attribute("active").as_bool(true);
   m_label = behaviour.attribute("label").as_string("");
   m_fuel.read(behaviour);

   m_base = getBody(props.attribute("base").as_string("stationBase"));
   m_arm = getBody(props.attribute("arm").as_string("dockArm"));
   m_clamp = getBody(props.attribute("clamp").as_string("dockClamp"));
   m_shoulder = dynamic_cast<b2RevoluteJoint*>(getJoint(props.attribute("shoulder").as_string("shoulderJoint")));

   m_clampPoint.Set(props.attribute("clampX").as_float(0.0f), props.attribute("clampY").as_float(0.0f));
   m_nosePoint.Set(props.attribute("noseX").as_float(0.0f), props.attribute("noseY").as_float(-5.25f));
   m_dockAngle = props.attribute("dockAngle").as_float(0.0f) * c_pi / 180.0f;

   m_captureRadius = props.attribute("captureRadius").as_float(16.0f);
   m_pullGain = props.attribute("pullGain").as_float(2.0f);
   m_pullSpeed = props.attribute("pullSpeed").as_float(4.0f);
   m_pullForce = props.attribute("pullForce").as_float(2600.0f);
   m_maxForce = props.attribute("maxForce").as_float(8000.0f);
   m_alignGain = props.attribute("alignGain").as_float(2.5f);
   m_maxAlignRate = props.attribute("maxAlignRate").as_float(1.2f);
   m_maxCaptureSpeed = props.attribute("maxCaptureSpeed").as_float(5.0f);
   m_maxCaptureAngle = props.attribute("maxCaptureAngle").as_float(25.0f) * c_pi / 180.0f;
   m_weldDistance = props.attribute("weldDistance").as_float(0.3f);
   m_weldSpeed = props.attribute("weldSpeed").as_float(1.0f);
   m_weldAngle = props.attribute("weldAngle").as_float(1.5f) * c_pi / 180.0f;
   m_captureTimeout = props.attribute("captureTimeout").as_float(10.0f);
   m_rejectTime = props.attribute("rejectTime").as_float(1.5f);

   m_armAwayAngle = props.attribute("armAwayAngle").as_float(-70.0f) * c_pi / 180.0f;
   m_armSpeed = props.attribute("armSpeed").as_float(70.0f) * c_pi / 180.0f;
   m_armTorque = props.attribute("armTorque").as_float(2000000.0f);
   m_armReturnDelay = props.attribute("armReturnDelay").as_float(3.0f);
   m_clearDistance = props.attribute("clearDistance").as_float(m_captureRadius + 12.0f);

   m_dockedVisibleWidth = props.attribute("visibleWidth").as_float(160.0f);

   // Station keeping: a motor joint to a static anchor where the station
   // starts, with limited force and torque
   if (m_base != NULL)
   {
      b2BodyDef anchorDef;
      anchorDef.type = b2_staticBody;
      anchorDef.position = m_base->GetPosition();
      anchorDef.angle = m_base->GetAngle();
      m_anchor = world->CreateBody(&anchorDef);

      m_base->SetAngularDamping(props.attribute("angularDamping").as_float(0.5f));

      b2MotorJointDef keepDef;
      keepDef.Initialize(m_anchor, m_base);
      keepDef.maxForce = props.attribute("keepForce").as_float(8000.0f);
      keepDef.maxTorque = props.attribute("keepTorque").as_float(300000.0f);
      keepDef.correctionFactor = props.attribute("keepCorrection").as_float(0.15f);
      keepDef.collideConnected = false;
      m_keeper = (b2MotorJoint*)world->CreateJoint(&keepDef);
   }
   else
   {
      logs::messageln("Docking station %s: no base body", getName().c_str());
   }

   if ((m_arm == NULL) || (m_clamp == NULL) || (m_shoulder == NULL))
   {
      logs::messageln("Docking station %s: arm, clamp or shoulder joint missing", getName().c_str());
   }
   else
   {
      // The arm rests at the shoulder's starting angle, held there rigidly
      m_shoulder->SetLimits(0.0f, 0.0f);
      m_shoulder->EnableLimit(true);
      m_shoulder->EnableMotor(false);
   }

   m_properties.push_back(ObjectProperty(this, NULL, propState, (float)m_state, true));
   m_properties.push_back(ObjectProperty(this, NULL, propLfDocked, 0.0f, true));
   m_properties.push_back(ObjectProperty(this, NULL, propActive, m_active ? 1.0f : 0.0f));
   m_properties.push_back(ObjectProperty(this, NULL, propFuelSupply, m_fuel.m_supply));
   m_properties.push_back(ObjectProperty(this, NULL, propFuelService, (float)m_fuel.m_service));

   // As for a landing pad: the CompoundObject's parent is m_parentObject
   setUserData(NULL);
   attachTo(sceneParent);

   if (m_shapes.size() > 0)
   {
      g_HeadDownDisplay->addMeToMap(
         MapItem::MapItemTypeEnum::neutralStationary,
         m_shapes[0],
         MapItem::MapItemStateEnum::hollow);
   }
}

// ---------------------------------------------------------------------------
// Contacts

bool DockingStation::armPassesLeapfrog(void) const
{
   return (m_state == capturing) || (m_state == docked) || (m_state == undocking);
}

void DockingStation::preSolve(b2Contact* contact)
{
   b2Fixture* fa = contact->GetFixtureA();
   b2Fixture* fb = contact->GetFixtureB();
   CollisionEntity::CollisionEntityTypeEnum ea = BodyUserData::getCollisionType(fa->GetUserData());
   CollisionEntity::CollisionEntityTypeEnum eb = BodyUserData::getCollisionType(fb->GetUserData());

   b2Fixture* armFixture = NULL;
   CollisionEntity::CollisionEntityTypeEnum other = CollisionEntity::notApplicable;

   if (ea == CollisionEntity::dockArm)
   {
      armFixture = fa;
      other = eb;
   }
   else if (eb == CollisionEntity::dockArm)
   {
      armFixture = fb;
      other = ea;
   }
   else
   {
      return;
   }

   // The Leapfrog's shield never touches the arm or the clamp
   if (other == CollisionEntity::shield)
   {
      contact->SetEnabled(false);
      return;
   }

   bool leapfrogPart =
      (other == CollisionEntity::leapfrog) ||
      (other == CollisionEntity::lfBigLeg) ||
      (other == CollisionEntity::lfSmallLeg) ||
      (other == CollisionEntity::lfBooster) ||
      (other == CollisionEntity::lfLeftFoot) ||
      (other == CollisionEntity::lfRightFoot) ||
      (other == CollisionEntity::lfSteerBooster);

   if (!leapfrogPart)
   {
      return;
   }

   void* ud = armFixture->GetBody()->GetUserData();
   if (ud == NULL)
   {
      return;
   }

   DockingStation* station = BodyUserData::getParentObjectOfType<DockingStation*>(ud);
   if ((station != NULL) && station->armPassesLeapfrog())
   {
      contact->SetEnabled(false);
   }
}

// ---------------------------------------------------------------------------
// Helpers

LeapFrog* DockingStation::getLeapfrog(void)
{
   LeapFrog* lf = (m_sceneActor != NULL) ? m_sceneActor->m_leapfrog : NULL;

   if (lf && lf->m_resources && !lf->m_isDead)
   {
      return lf;
   }

   return NULL;
}

b2Body* DockingStation::getLeapfrogBody(LeapFrog* lf)
{
   return (lf != NULL) ? lf->getBody("lfMainBody") : NULL;
}

bool DockingStation::jointExists(b2Joint* joint)
{
   for (b2Joint* j = m_world->GetJointList(); j != NULL; j = j->GetNext())
   {
      if (j == joint)
      {
         return true;
      }
   }
   return false;
}

void DockingStation::setState(StationState state)
{
   m_state = state;
   m_stateTime = 0.0f;
   m_properties[propState].setProperty((float)state);
}

// How far the Leapfrog's attitude is from the docked attitude [rad]
float DockingStation::attitudeError(b2Body* lfBody)
{
   return wrapAngle(lfBody->GetAngle() - m_clamp->GetAngle() - m_dockAngle);
}

void DockingStation::moveArm(float target)
{
   if (m_shoulder == NULL)
   {
      return;
   }

   m_armTarget = target;
   m_armMoving = true;
   m_shoulder->EnableLimit(false);
   m_shoulder->SetMaxMotorTorque(m_armTorque);
   m_shoulder->EnableMotor(true);
}

// The shoulder motor drives the arm to its target, slowing down near it,
// then the joint is locked there
void DockingStation::updateArm(void)
{
   if ((m_shoulder == NULL) || !m_armMoving)
   {
      return;
   }

   float error = m_armTarget - m_shoulder->GetJointAngle();

   if (fabs(error) < 0.01f)
   {
      m_shoulder->SetMotorSpeed(0.0f);
      m_shoulder->EnableMotor(false);
      m_shoulder->SetLimits(m_armTarget, m_armTarget);
      m_shoulder->EnableLimit(true);
      m_armMoving = false;
      return;
   }

   m_shoulder->SetMotorSpeed(clampf(3.0f * error, -m_armSpeed, m_armSpeed));
}

// The magnet draws the Leapfrog's nose to the clamp at a closing speed
// that falls off near it (pullGain per unit of distance, at most
// pullSpeed), and turns the Leapfrog towards the clamp's attitude. In deep
// space the Leapfrog's turn rate is set directly (steering sets it every
// update), so the station sets it too, after the controls, before the next
// step. The reaction pulls the station the other way.
void DockingStation::applyMagnet(b2Body* lfBody, float dt)
{
   b2Vec2 clampWorld = m_clamp->GetWorldPoint(m_clampPoint);
   b2Vec2 noseWorld = lfBody->GetWorldPoint(m_nosePoint);

   b2Vec2 relVel = lfBody->GetLinearVelocityFromWorldPoint(noseWorld) -
      m_clamp->GetLinearVelocityFromWorldPoint(clampWorld);

   b2Vec2 wanted = m_pullGain * (clampWorld - noseWorld);
   float wantedSpeed = wanted.Length();
   if (wantedSpeed > m_pullSpeed)
   {
      wanted *= m_pullSpeed / wantedSpeed;
   }

   b2Vec2 force = m_pullForce * (wanted - relVel);
   float mag = force.Length();
   if (mag > m_maxForce)
   {
      force *= m_maxForce / mag;
   }

   lfBody->ApplyForce(force, noseWorld, true);
   m_clamp->ApplyForce(-force, clampWorld, true);

   float rate = clampf(-m_alignGain * attitudeError(lfBody), -m_maxAlignRate, m_maxAlignRate);
   lfBody->SetAngularVelocity(m_clamp->GetAngularVelocity() + rate);
}

void DockingStation::dock(LeapFrog* lf, b2Body* lfBody)
{
   // Seat the nose exactly in the clamp (it is at most weldDistance and
   // weldAngle off; the legs and the shield follow through their joints)
   float angle = m_clamp->GetAngle() + m_dockAngle;
   b2Vec2 seat = m_clamp->GetWorldPoint(m_clampPoint) - b2Mul(b2Rot(angle), m_nosePoint);
   lfBody->SetTransform(seat, angle);

   b2WeldJointDef def;
   def.bodyA = m_clamp;
   def.bodyB = lfBody;
   def.localAnchorA = m_clampPoint;
   def.localAnchorB = m_nosePoint;
   def.referenceAngle = m_dockAngle;
   def.collideConnected = false;
   m_dockJoint = (b2WeldJoint*)m_world->CreateJoint(&def);

   // No relative motion left
   b2Vec2 clampWorld = m_clamp->GetWorldPoint(m_clampPoint);
   lfBody->SetLinearVelocity(m_clamp->GetLinearVelocityFromWorldPoint(clampWorld));
   lfBody->SetAngularVelocity(m_clamp->GetAngularVelocity());

   setState(docked);
   m_properties[propLfDocked].setProperty(1.0f);
   m_boosterReleased = !lf->isMainBoosterFiring();
   m_fuel.m_messageShown = false;

   if (lf->m_shield)
   {
      lf->m_shield->setHeldUp(true);
   }

   if (m_sceneActor != NULL)
   {
      m_sceneActor->setLeapfrogDocked(this, true);
      m_sceneActor->setLeapfrogStandsOn(this, true);

      if (m_dockedVisibleWidth > 0.0f)
      {
         m_sceneActor->zoomInToVisibleWidth(m_dockedVisibleWidth, c_dockZoomMs);
      }
   }

   if (g_HeadDownDisplay)
   {
      g_HeadDownDisplay->logMessage(m_label.empty() ? string("Docked") : ("Docked at " + m_label));
   }

   logs::messageln("Docking station %s: docked", getName().c_str());

   m_dockedSent = false;
   m_refusalShown = false;
   if (m_active)
   {
      m_dockedSent = true;
      DockingStationDockedEvent event;
      dispatchEvent(&event);
   }
}

void DockingStation::undock(LeapFrog* lf, bool zoom)
{
   if ((m_dockJoint != NULL) && jointExists(m_dockJoint))
   {
      m_world->DestroyJoint(m_dockJoint);
   }
   m_dockJoint = NULL;

   if ((lf != NULL) && lf->m_shield)
   {
      lf->m_shield->setHeldUp(false);
   }

   if (m_sceneActor != NULL)
   {
      m_sceneActor->setLeapfrogDocked(this, false);
      m_sceneActor->setLeapfrogStandsOn(this, false);

      if (zoom && (m_dockedVisibleWidth > 0.0f))
      {
         m_sceneActor->zoomOutToDefault(c_undockZoomMs);
      }
   }

   m_properties[propLfDocked].setProperty(0.0f);
   m_announceDocked = false;
   setState(undocking);
   moveArm(m_armAwayAngle);

   logs::messageln("Docking station %s: undocked", getName().c_str());

   if (m_dockedSent)
   {
      m_dockedSent = false;
      DockingStationUndockedEvent event;
      dispatchEvent(&event);
   }
}

void DockingStation::refuse(const char* why)
{
   setState(refused);

   // Told once per approach: hovering just inside the radius would
   // otherwise repeat it every rejectTime
   if (!m_refusalShown && g_HeadDownDisplay)
   {
      g_HeadDownDisplay->logMessage(string("Docking refused: ") + why);
      m_refusalShown = true;
   }

   logs::messageln("Docking station %s: refused (%s)", getName().c_str(), why);
}

// ---------------------------------------------------------------------------
// Update

void DockingStation::doUpdate(const UpdateState& us)
{
   if ((m_clamp == NULL) || (m_shoulder == NULL) || g_MainActor->isInPause())
   {
      return;
   }

   float dt = us.dtf;
   m_stateTime += dt;

   updateArm();

   LeapFrog* lf = getLeapfrog();
   b2Body* lfBody = getLeapfrogBody(lf);

   b2Vec2 clampWorld = m_clamp->GetWorldPoint(m_clampPoint);
   float distance = 1.0e9f;
   if (lfBody != NULL)
   {
      distance = (lfBody->GetWorldPoint(m_nosePoint) - clampWorld).Length();
   }

   switch (m_state)
   {
   case ready:
      // A new approach: a refusal is told again
      if (distance > m_captureRadius * 1.25f)
      {
         m_refusalShown = false;
      }

      // Nothing is caught while the arm is on its way back
      if ((lfBody != NULL) && !m_armMoving && (distance < m_captureRadius))
      {
         b2Vec2 noseWorld = lfBody->GetWorldPoint(m_nosePoint);
         b2Vec2 relVel = lfBody->GetLinearVelocityFromWorldPoint(noseWorld) -
            m_clamp->GetLinearVelocityFromWorldPoint(clampWorld);

         if (relVel.Length() > m_maxCaptureSpeed)
         {
            refuse("too fast");
         }
         else if (fabs(attitudeError(lfBody)) > m_maxCaptureAngle)
         {
            refuse("not lined up");
         }
         else
         {
            setState(capturing);
            logs::messageln("Docking station %s: capturing", getName().c_str());
         }
      }
      break;

   case capturing:
   {
      if (lfBody == NULL)
      {
         setState(ready);
         break;
      }

      // Got away (e.g. boosted out of it)
      if (distance > m_captureRadius * 1.25f)
      {
         setState(ready);
         break;
      }

      if (m_stateTime > m_captureTimeout)
      {
         refuse("no grip");
         break;
      }

      b2Vec2 noseWorld = lfBody->GetWorldPoint(m_nosePoint);
      b2Vec2 relVel = lfBody->GetLinearVelocityFromWorldPoint(noseWorld) -
         m_clamp->GetLinearVelocityFromWorldPoint(clampWorld);

      if ((distance < m_weldDistance) &&
         (relVel.Length() < m_weldSpeed) &&
         (fabs(attitudeError(lfBody)) < m_weldAngle))
      {
         dock(lf, lfBody);
      }
      else
      {
         applyMagnet(lfBody, dt);
      }
      break;
   }

   case docked:
      if ((lf == NULL) || (lfBody == NULL) || (m_dockJoint == NULL) || !jointExists(m_dockJoint))
      {
         m_dockJoint = NULL;
         undock(lf, true);
         break;
      }

      // A new press of the booster undocks (not one held since docking)
      if (!lf->isMainBoosterFiring())
      {
         m_boosterReleased = true;
      }
      else if (m_boosterReleased)
      {
         undock(lf, true);
         break;
      }

      // Held at the clamp's attitude (steering would stop its turning
      // every update and fight the weld)
      lfBody->SetAngularVelocity(m_clamp->GetAngularVelocity());

      if (m_announceDocked)
      {
         m_announceDocked = false;
         m_dockedSent = true;
         DockingStationDockedEvent event;
         dispatchEvent(&event);
      }

      if (m_active)
      {
         if (m_fuel.refuel(lf->m_resources, dt))
         {
            m_properties[propFuelSupply].setProperty(m_fuel.m_supply);
         }
      }

      if (m_sceneActor != NULL)
      {
         m_sceneActor->setLeapfrogStandsOn(this, true);
      }
      break;

   case undocking:
      // The arm comes back once the Leapfrog is clear of it
      if ((m_stateTime > m_armReturnDelay) && (distance > m_clearDistance))
      {
         moveArm(0.0f);
         setState(ready);
      }
      break;

   case refused:
      if (m_stateTime > m_rejectTime)
      {
         setState(ready);
      }
      break;
   }
}

void DockingStation::setPropertyImpl(int propId, float value)
{
   if (propId == propActive)
   {
      bool wasActive = m_active;
      m_active = (value != 0.0f);

      // Activated while docked: as if it had just docked. Sent from
      // doUpdate, so the event doesn't reach Lua from inside its own
      // c_setObjectProperty call.
      m_announceDocked = m_active && !wasActive && (m_state == docked) && !m_dockedSent;

      logs::messageln("Docking station %s: %s", getName().c_str(), m_active ? "active" : "inactive");
   }
   else if (propId == propFuelSupply)
   {
      m_fuel.m_supply = value;
      m_fuel.m_messageShown = false;
   }
   else if (propId == propFuelService)
   {
      m_fuel.setService(value);
   }
}
