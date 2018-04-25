#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "asteroid.h"

class FreeSpaceActor;

DECLARE_SMART(AsteroidField, spAsteroidField);

class AsteroidField : public CompoundObject
{
private:
   oxygine::Resources* m_gameResources;
   oxygine::Actor* m_parent;
   b2World* m_world;
   FreeSpaceActor* m_spaceActor;
   SceneActor* m_sceneActor;


   b2Vec2 m_leftTop;
   b2Vec2 m_rightBottom;

   int m_initialSpawn;

   // Time between spawned asteroids
   float m_interval;

   // The time, from the creation until the field stop spawning new asteroids
	int m_fieldLifetime; // [ms}


   int m_timeSinceLast;

   std::vector<AsteroidSpawnInstruction>  m_asteroidSpawnList;
   
   void readAsteroidFieldNode(pugi::xml_node& objectNode);
   void spawnAsteroids(void);
   
public:
   AsteroidField(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      pugi::xml_node& objectNode);

   void addAsteroidSpawnInstruction(AsteroidSpawnInstruction& inst);

   virtual CollisionEntityTypeEnum getEntityType(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
