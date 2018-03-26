#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "softboundary.h"
#include "freespacecontactlistener.h"
#include "asteroid.h"


DECLARE_SMART(FreeSpaceActor, spFreeSpaceActor);

class FreeSpaceActor : public SceneActor
{
private:
   spSoftBoundary m_lowerBoundary;
   spSoftBoundary m_leftBoundary;
   spSoftBoundary m_rightBoundary;
   spSoftBoundary m_topBoundary;

   std::vector<b2Body*>   m_boundedBodies;
   std::vector<AsteroidSpawnInstruction>  m_asteroidSpawnList;

   FreeSpaceContactListener   m_contactListener;

public:
	FreeSpaceActor(oxygine::Resources& gameResources);

   void addBoundingBody(oxygine::Actor* body);
   void removeBoundingBody(oxygine::Actor* body);
   void testForBoundaryRepel(void);
   void addAsteroidSpawnInstruction(AsteroidSpawnInstruction& inst);


protected:
   void doUpdate(const oxygine::UpdateState &us);
   void spawnAsteroids(void);
   void generateBackground(Resources& gameResources);

};
