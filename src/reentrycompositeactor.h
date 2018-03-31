#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "planetactor.h"
#include "leapfrog.h"

enum ReentryStateEnum
{
   RES_INITITAL,
   RES_WAIT_FOR_BREAK,
   RES_REENTRY,
   RE_FINISHED
};

DECLARE_SMART(ReentryCompositeActor, spReentryCompositeActor);

class ReentryCompositeActor : public Actor
{
private:
   spPlanetActor m_planetActor;
   b2World * m_world;
   spLeapFrog m_leapFrog;
   spActor m_lfStage;
   bool m_burnedLastUpdate;
   timeMS m_reentryStartTime;
   ReentryStateEnum m_state;


public:
   ReentryCompositeActor(oxygine::Resources& gameResources);
   void doUpdate(const UpdateState& us);

   void onWaitDone(Event* event);
   void onCenterDone(Event* event);
   void onBurnDone(Event* event);
   void onRotateDone(Event* event);
   void onModeDone(Event* event);


};
