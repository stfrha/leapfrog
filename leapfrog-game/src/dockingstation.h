#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"
#include "landingpad.h"
#include <string>

class SceneActor;
class LeapFrog;

DECLARE_SMART(DockingStation, spDockingStation);

/*
A docking station: the space version of a landing pad.

The station is a compound object of three bodies (names set in
<dockingStationProperties>): the station's body (base), an arm on a
shoulder joint (revolute, base to arm) and a clamp welded to the arm's tip
(a weldJoint in the XML). The arm and the clamp use the collision entity
"dockArm".

- Station keeping: the base is tied to a static anchor at its starting
  place with a motor joint of limited force and torque, so knocks move it
  and it slowly drifts back.
- Magnet: when the Leapfrog's nose comes within captureRadius of the
  clamp, slowly enough (maxCaptureSpeed) and lined up (maxCaptureAngle),
  the magnet draws the nose to the clamp (at most pullSpeed, slowing down
  near it) and the station turns the Leapfrog to the clamp's attitude. Too fast or not lined up: no grab, the
  magnet is off for rejectTime (the hit just knocks the station).
- Docked: when the nose is at the clamp, slow and lined up, it is welded
  there. The Leapfrog's shield is held up by the station (on, and hits
  don't drain it). Only the booster works; a fresh press undocks: the weld
  goes, the arm swings up and away (armAwayAngle) and the Leapfrog passes
  through it. The arm comes back once the Leapfrog is clear.
- The arm and clamp never touch the Leapfrog's shield, and don't touch the
  Leapfrog itself while it is being captured, docked or leaving.
- Docked counts as standing on the station for questions (SceneActor),
  and fuel flows as at a landing pad (fuel, fuelRate, fuelPrice,
  fuelSupply on <behaviour>, see landingpad.h).

Events: Leapfrog docked (DsDk) and undocked (DsUd), only while active.

Properties (Lua):
- 0 state: 0 ready, 1 capturing, 2 docked, 3 undocking (arm away),
  4 refused (magnet off for a while) (read only)
- 1 lfDocked: 1 while the Leapfrog is docked (read only)
- 2 active: as for a landing pad: an inactive station still docks, but
  sends no events and gives no fuel
- 3 fuelSupply, 4 fuelService: as for a landing pad
*/

class DockingStationDockedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('D', 's', 'D', 'k') };
   DockingStationDockedEvent(void) : Event(EVENT) {}
};

class DockingStationUndockedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('D', 's', 'U', 'd') };
   DockingStationUndockedEvent(void) : Event(EVENT) {}
};

class DockingStation : public CompoundObject
{
public:
   enum propertyId
   {
      propState = 0,
      propLfDocked = 1,
      propActive = 2,
      propFuelSupply = 3,
      propFuelService = 4
   };

   enum StationState
   {
      ready = 0,
      capturing = 1,
      docked = 2,
      undocking = 3,
      refused = 4
   };

private:
   b2World* m_world;
   b2Body* m_base;
   b2Body* m_arm;
   b2Body* m_clamp;
   b2Body* m_anchor;
   b2MotorJoint* m_keeper;
   b2RevoluteJoint* m_shoulder;
   b2WeldJoint* m_dockJoint;

   StationState m_state;
   float m_stateTime;            // [s] in the current state
   bool m_active;
   bool m_boosterReleased;       // docked: the booster has been up since docking
   bool m_dockedSent;            // DsDk sent for this docking
   bool m_announceDocked;        // activated while docked: send DsDk in doUpdate
   bool m_refusalShown;          // "Docking refused" logged for this approach
   std::string m_label;
   PadFuel m_fuel;

   // The clamp's grip point (clamp body coordinates) and the matching point
   // on the Leapfrog's main body (the recess in its nose)
   b2Vec2 m_clampPoint;
   b2Vec2 m_nosePoint;
   float m_dockAngle;            // Leapfrog angle minus clamp angle when docked

   // Magnet
   float m_captureRadius;
   float m_pullGain;             // wanted closing speed per unit of distance [1/s]
   float m_pullSpeed;            // highest closing speed [units/s]
   float m_pullForce;            // N per unit/s of speed error
   float m_maxForce;             // N
   float m_alignGain;            // rad/s per rad of attitude error
   float m_maxAlignRate;         // rad/s
   float m_maxCaptureSpeed;      // units/s
   float m_maxCaptureAngle;      // rad
   float m_weldDistance;
   float m_weldSpeed;
   float m_weldAngle;            // rad
   float m_captureTimeout;       // s
   float m_rejectTime;           // s

   // Arm
   float m_armTarget;            // shoulder angle the arm is going to
   bool m_armMoving;
   float m_armAwayAngle;         // rad
   float m_armSpeed;             // rad/s
   float m_armTorque;
   float m_armReturnDelay;       // s after undocking
   float m_clearDistance;        // the arm comes back when the nose is this far

   float m_dockedVisibleWidth;   // zoom while docked (0 = no zoom)

   LeapFrog* getLeapfrog(void);
   b2Body* getLeapfrogBody(LeapFrog* lf);
   bool jointExists(b2Joint* joint);
   void setState(StationState state);
   void moveArm(float target);
   void updateArm(void);
   float attitudeError(b2Body* lfBody);
   void applyMagnet(b2Body* lfBody, float dt);
   void dock(LeapFrog* lf, b2Body* lfBody);
   void undock(LeapFrog* lf, bool zoom);
   void refuse(const char* why);

public:
   DockingStation(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

   // Contact filter, called from both contact listeners' PreSolve
   static void preSolve(b2Contact* contact);

   // Should contacts between the arm/clamp and the Leapfrog be ignored now?
   bool armPassesLeapfrog(void) const;

   bool isActive(void) const { return m_active; }

protected:
   void doUpdate(const oxygine::UpdateState& us) override;
   void setPropertyImpl(int propId, float value) override;
};
