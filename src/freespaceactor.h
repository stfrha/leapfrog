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
public:
   enum States
   {
      insertBurn,
      operate
   };


private:
   spSoftBoundary m_lowerBoundary;
   spSoftBoundary m_leftBoundary;
   spSoftBoundary m_rightBoundary;
   spSoftBoundary m_topBoundary;

   States m_state;
   int m_stateChangeTime;

   bool m_inOrbitField;
   oxygine::timeMS m_enteredOrbitFieldAtTime;

   b2Body* m_leapfrogBody;

   std::vector<b2Body*>   m_boundedBodies;

   FreeSpaceContactListener   m_contactListener;

public:
	FreeSpaceActor(
      oxygine::Resources& gameResources,
      const std::string& fileName,
      const std::string& initialState);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void addBoundingBody(b2Body* body);
   void removeBoundingBody(b2Body* body);
   void testForBoundaryRepel(void);
   bool isInsideOrbitField(b2Body* body);



protected:
   void doUpdate(const oxygine::UpdateState &us);
   void generateBackground(Resources& gameResources);

};
