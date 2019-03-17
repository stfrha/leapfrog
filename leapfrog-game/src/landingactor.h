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





   void leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev);
   void transitToDeepSpace(oxygine::Event *ev);



public:
	LandingActor(
      oxygine::Resources& gameResources, 
      b2World* world,
      pugi::xml_node& root,
      const std::string& initialState,
      int groupIndex);

};
