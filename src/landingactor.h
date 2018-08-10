#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "landercontactlistener.h"
#include "landingactorevents.h"

DECLARE_SMART(LandingActor, spLandingActor);

class LandingActor : public SceneActor
{
public:
   //enum propertyId
   //{
   //   state = 0,
   //   lfLanded = 1
   //};



private:
   LanderContactListener   m_contactListener;





   void modeReachedListener(oxygine::Event *ev);
   void leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev);
   void handlePropertyTriggeredEvent(oxygine::Event *ev);
   void transitToDeepSpace(oxygine::Event *ev);



public:
	LandingActor(oxygine::Resources& gameResources, const std::string& fileName, const std::string& initialState);

   virtual CollisionEntityTypeEnum getEntityType(void);

};
