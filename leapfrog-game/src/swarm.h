#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "spawnobject.h"

class SceneActor;

DECLARE_SMART(Swarm, spSwarm);

/*
A Swarm spawns enemies (e.g. strippers) from all sides of the scene: each
one appears at a random spot on a random side, in the soft boundary haze
just outside the playfield (spawnInset outside the scene edge), and not
closer than minDistance to the Leapfrog. The spawned objects fly in by
themselves (strippers are not pushed by the boundaries).

It is a named object, so Lua can control it, e.g. the scene XML has
   <childObject name="swarm" file="stripper_swarm.xml" />
or a mission adds it: c_addPositionedChildObject("stripper_swarm.xml", "swarm", 0, 0)

Rate and size: one spawn every 1 / rate s on average (all sides together:
four sides each get a quarter), while fewer than maxAlive of its spawns are
alive. Both ramp up linearly from startRate / startMaxAlive to rate /
maxAlive over rampTime s of being active (0 = at once). Setting rate or
maxAlive from Lua ends the ramp: Lua is in charge from then on.

XML (behaviour type "swarm", node swarmProperties):
   spawn         files to spawn, separated by spaces, one picked at random
                 per spawn (name a file twice to make it twice as likely)
   active        starts spawning at once (default true)
   rate, startRate, maxAlive, startMaxAlive, rampTime
   initialSpawn  spawned at once when it becomes active the first time
   sides         which sides: any of "up down left right" (default all
                 that have a soft boundary; all four if the scene has none)
   spawnInset    how far outside the scene edge (into the haze) they appear
   minDistance   not closer to the Leapfrog than this

Properties (Lua):
   0 active      1 = spawning, 0 = stopped (the live ones carry on)
   1 rate        spawns per second, now
   2 maxAlive    most alive at a time, now
   3 alive       read only: its spawns alive now
   4 spawned     read only: spawned so far
   5 recall      write 1: every live stripper of the swarm gives up and
                 leaves (fades out); reads 0
*/

class Swarm : public CompoundObject
{
public:
   enum SwarmPropertyEnum
   {
      propActive = 0,
      propRate = 1,
      propMaxAlive = 2,
      propAlive = 3,
      propSpawned = 4,
      propRecall = 5
   };

   enum SideEnum
   {
      sideUp = 0,       // top edge
      sideDown = 1,     // bottom edge
      sideLeft = 2,
      sideRight = 3
   };

private:
   b2World* m_world;
   spSpawnObjectList m_spawnObjects;
   spSpawnTally m_tally;

   bool m_active;
   bool m_ramping;
   bool m_initialDone;
   float m_rampClock;            // s active so far
   float m_untilNext;            // s to the next spawn

   float m_rate;                 // spawns/s now
   float m_maxAlive;             // now
   float m_startRate;
   float m_endRate;
   float m_startMaxAlive;
   float m_endMaxAlive;
   float m_rampTime;
   int m_initialSpawn;
   int m_spawned;

   std::vector<int> m_sides;
   float m_spawnInset;
   float m_minDistance;

   void readSwarmNode(const pugi::xml_node& node);
   void readSides(const std::string& sides);
   void updateRamp(float dt);
   void scheduleNext(void);
   bool spawnOne(void);
   bool pickSpawnPoint(oxygine::Vector2& pos);
   void recall(void);

protected:
   void setPropertyImpl(int propId, float value) override;
   void doUpdate(const oxygine::UpdateState& us) override;

public:
   Swarm(
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);
};
