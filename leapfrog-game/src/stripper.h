#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "spawnobject.h"

class SceneActor;
class LeapFrog;

DECLARE_SMART(Stripper, spStripper);

/*
A Stripper is a small enemy that strips the Leapfrog of what it carries.
Many of them together overwhelm it.

What it does:
1. approach: flies at full speed to where it can meet the Leapfrog (the
   intercept point, at most lead s ahead; head-on that is straight into
   its path). Within brakeDistance it moves sideways with the Leapfrog and
   closes in on it; if the Leapfrog is coming at it faster than that, it
   holds its ground and grabs it as it arrives.
   While the Leapfrog's shield is up it rams the shield at ramSpeed and
   bounces back (stunned) to come again: each ram drains shieldDrain from
   the shield (plus what the knock itself drains).
2. latched: with the shield down it closes in at latchSpeed and grabs the
   hull when it is within latchDistance of it (no collision, so no hull
   damage), welded where it is. It adds its weight to the Leapfrog (to a
   leg if it grabbed a leg, which then carries it). It works for
   stealTime seconds (sparks fly at the grip point every sparkInterval).
   It loses its grip, and is stunned for stunTime, if:
   - the weld has to carry more than gripForce for gripTime s in a row
     (hard manoeuvres shake it off; gripForce 0 = never), or
   - the Leapfrog's shield comes back up.
3. When the time is up it takes ONE thing: one of the resources listed in
   "steal" that the Leapfrog has some of, picked at random (the amount for
   each resource is an attribute). The HUD logs it and the Leapfrog sends
   the event "LfSt" to Lua with the resource's Leapfrog property id as
   parameter (5 ammo, 6 shield, 7 fuel, 8 credits). With nothing left to
   take it sends LfSt with parameter 0 (lua/swarm.lua uses that).
4. leave: flies away from the Leapfrog and fades out over leaveTime, then
   it is gone. With a swarm (or an objectFactory with maxAlive), its place
   is then taken by a new one.
It is not pushed by the soft boundaries (bounded="true" to make it), so a
swarm can spawn it in the boundary haze; it flies in by itself.
If it is shot down (bullets, blasts) while carrying loot, it drops the
compound object in dropFile (a pickupObject), set to give back exactly
what it took (pickupType and amount are overwritten), so the player can
win it back.

Properties (Lua, read only): 0 state (StripperState), 1 x, 2 y.

The look is in the XML only. latchedShape and flyingShape optionally name
shapes of the object that are shown only while latched / while not latched
(e.g. arms stabbing / arms folded).
*/

class Stripper : public CompoundObject
{
public:
   enum StripperState
   {
      approach = 0,
      latched = 1,
      stunned = 2,
      leave = 3,
      gone = 4
   };

   enum StripperPropertyEnum
   {
      propState = 0,
      propXPos = 1,
      propYPos = 2
   };

   // Leapfrog property ids, also used as the LfSt parameter
   enum LootEnum
   {
      lootNone = 0,
      lootAmmo = 5,
      lootShield = 6,
      lootFuel = 7,
      lootCredits = 8
   };

private:
   b2World* m_world;
   b2Body* m_body;

   StripperState m_state;
   float m_stateTime;            // s in the current state
   float m_sparkTime;            // s since the last spark while latched

   b2WeldJoint* m_weld;
   b2Vec2 m_gripLocal;           // grip point in the Leapfrog main body's frame
   b2Filter m_ownFilters[8];     // the stripper's own fixture filters while free
   int m_numOwnFilters;

   int m_loot;                   // LootEnum carried away
   float m_lootAmount;
   float m_health;
   bool m_shotDown;              // health gone, removed in the next update
   bool m_leaveRequested;        // leaveNow called
   int m_mapItemId;

   // From XML
   float m_maxSpeed;             // units/s
   float m_acceleration;         // units/s2
   float m_turnRate;             // rad/s
   float m_noseAngle;            // rad, body +x to the nose (arms)
   float m_lead;                 // s, max look-ahead to the meeting point
   float m_brakeDistance;        // units from the Leapfrog's centre where it starts to brake
   float m_latchSpeed;           // units/s, closing speed when it reaches the hull
   float m_latchDistance;        // units, gap to the hull at which it grabs it
   float m_ramSpeed;             // units/s, closing speed when it rams the shield
   float m_shieldDrain;          // shield taken by each ram (the knock drains a little more)
   float m_stealTime;            // s latched before it takes something
   float m_gripForce;            // N, 0 = never shaken off
   float m_gripTime;             // s above gripForce in a row before it lets go
   float m_strainTime;           // s above gripForce so far
   float m_stunTime;             // s
   float m_leaveTime;            // s
   float m_fadeInTime;           // s, when it appears
   float m_leaveSpeed;           // units/s
   float m_sparkInterval;        // s, 0 = no sparks
   bool m_antiGravity;           // hovers (cancels the world's gravity)
   bool m_bounded;               // pushed by the soft boundaries (default no)
   std::vector<int> m_stealList; // LootEnum
   float m_ammoAmount;
   float m_shieldAmount;
   float m_fuelAmount;
   float m_creditsAmount;
   std::string m_latchedShapeName;
   std::string m_flyingShapeName;
   std::string m_dropFile;

   void readStripperNode(const pugi::xml_node& node);
   void setState(StripperState state);

   LeapFrog* getLeapfrog(void);
   b2Body* getLeapfrogBody(LeapFrog* lf);

   bool shieldUp(LeapFrog* lf);
   bool weldIsValid(void);
   float reach(const b2Vec2& dir);
   bool findHull(LeapFrog* lf, b2Body* lfBody, b2Vec2& point, b2Body*& part);
   bool touchingShield(LeapFrog* lf);
   void latchOn(LeapFrog* lf, b2Body* lfBody, const b2Vec2& point);
   void letGo(void);
   void restoreFilters(void);
   int takeLoot(LeapFrog* lf);
   void steerTowards(const b2Vec2& desiredVelocity, float dt);
   void faceDirection(const b2Vec2& direction, float dt);
   void cancelGravity(void);
   void spark(const b2Vec2& worldPoint);
   void showStateShapes(void);
   void vanish(void);
   void dropLoot(void);
   void startLeaving(LeapFrog* lf, b2Body* lfBody);

public:
   Stripper(
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

   void hitByBullet(b2Contact* contact, float bulletEqvDamage);

   static const char* lootName(int loot);

   // Gives up and leaves now (fades out), at the next update. Safe to call
   // from anywhere, also during the physics step.
   void leaveNow(void);

   // For the contact listeners' PreSolve: strippers pass through each
   // other (one pushing into another must not shake it off the hull)
   static void preSolve(b2Contact* contact);

protected:
   void doUpdate(const oxygine::UpdateState& us) override;
};
