#pragma once
#include "oxygine-framework.h"
#include "Box2d/Box2D.h"
#include "sceneactor.h"
#include "softboundary.h"
#include "freespacecontactlistener.h"

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
   std::vector<b2Body*>   m_boundedBodies;

   FreeSpaceContactListener   m_contactListener;

public:
   b2Body * m_leapfrogBody;

   FreeSpaceActor(
      Resources& gameResources,
      b2World* world,
      pugi::xml_node& root,
      const std::string& initialState,
      int groupIndex);


   void addBoundingBody(b2Body* body);
   void removeBoundingBody(b2Body* body);
   void testForBoundaryRepel(void);
   bool isInsideOrbitField(b2Body* body);

protected:
   void doUpdate(const oxygine::UpdateState &us);
   void generateBackground(Resources& gameResources);

};
