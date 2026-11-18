#pragma once

#include "system.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"

DECLARE_SMART(ObjectFactory, spObjectFactory);

// Sent by the scene when a watched factory (c_watchFactory) has this few of
// its own objects left. Their pieces and drops don't count. Lua gets the
// watch's id as parameter.
class FactoryLowEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('F', 'c', 'L', 'w') };
   int m_watchId;

   FactoryLowEvent(int id) : Event(EVENT), m_watchId(id) {}
};

class CompoundObject;
class SceneActor;

class ObjectFactory : public System
{
private:
   // Parameters from XML
   b2Body * m_attachedBody;

   oxygine::Vector2 m_leftTop;
   oxygine::Vector2 m_widthHeight;

   // Time between spawned asteroids
   float m_interval;

   // The time, from the creation until the field stop spawning new asteroids
   int m_fieldLifetime; // [ms}

   int m_initialSpawn;

   // At most this many of its objects alive at a time (0 = no limit).
   // A new one is spawned at the normal interval only while below it,
   // so a swarm stays at this size: one that dies or leaves is replaced.
   int m_maxAlive;
   spSpawnTally m_tally;

   // At most this many objects in all (0 = no limit), then it stops. A
   // field that grows from a few rocks has a known size this way, so a
   // watch counts down from it.
   int m_spawnTotal;
   int m_spawnedSoFar;

   // Spawns at the intensity only while active. A factory that starts
   // inactive (active="false") does its spawnInitial when Lua first
   // activates it.
   bool m_active;
   bool m_initialPending;
   float m_intensity;          // spawns per second (0 = none at intervals)

   // watch(): send FactoryLowEvent when this few are left (-1 = no watch)
   int m_watchLeft;
   int m_watchId;

   // Working member variable
   pugi::xml_document* m_coNodeHolder;
   pugi::xml_node m_coNode;
   float m_timeSinceLast;      // ms
   spSpawnObjectList m_spawnObjects;

   void readObjectFactoryNode(const pugi::xml_node& objectNode);
   void spawnObjects(int num);
   void setIntensity(float perSecond);
   
public:
   ObjectFactory(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode);

   ~ObjectFactory();

   // Sends one FactoryLowEvent with the id, when at most left objects of
   // this factory are alive. With a spawnTotal, not before all of them
   // have been spawned.
   void watch(int left, int id);

   // Lua control (c_setFactoryProperty / c_getFactoryProperty):
   // 0 active (0/1), 1 intensity (per second), 2 maxAlive, 3 spawnTotal,
   // 4 alive (read only), 5 spawned (counts toward spawnTotal; write 0 to
   // start a new round), 6 spawn now (write n; within maxAlive and
   // spawnTotal, also when inactive; reads 0)
   enum PropertyEnum
   {
      propActive = 0,
      propIntensity = 1,
      propMaxAlive = 2,
      propSpawnTotal = 3,
      propAlive = 4,
      propSpawned = 5,
      propSpawnNow = 6
   };

   bool setFactoryProperty(int id, float value);
   bool getFactoryProperty(int id, float& value);

protected:
	void doUpdate(const oxygine::UpdateState& us) override;

};
