#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "landercontactlistener.h"
#include "landingactorevents.h"

DECLARE_SMART(LandingActor, spLandingActor);

class LandingActor : public SceneActor
{
private:
   oxygine::Resources* m_gameResources;
   LanderContactListener   m_contactListener;

   void modeReachedListener(oxygine::Event *ev);
   void leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev);
   void handlePropertyTriggeredEvent(oxygine::Event *ev);
   void transitToDeepSpace(oxygine::Event *ev);

public:
	LandingActor(oxygine::Resources& gameResources);

   void initLandingActor(
      std::string& fileName,
      std::string& initialState);


   virtual CollisionEntityTypeEnum getEntityType(void);

};
