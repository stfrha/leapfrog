#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "softboundary.h"


DECLARE_SMART(FreeSpaceActor, spFreeSpaceActor);

class FreeSpaceActor : public SceneActor
{
private:
   spSoftBoundary m_lowerBoundary;
   spSoftBoundary m_leftBoundary;
   spSoftBoundary m_rightBoundary;
   spSoftBoundary m_topBoundary;

   std::vector<b2Body*>   m_boundedBodies;

public:
	FreeSpaceActor(oxygine::Resources& gameResources);

   void testForBoundaryRepel(void);

protected:
   void doUpdate(const oxygine::UpdateState &us);

};
