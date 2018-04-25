#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"

DECLARE_SMART(LaunchSite, spLaunchSite);

/*
Available events:
- leapfrog landed on leg rest 
- isPrepared
- launchSequenceComplete
- hasLaunched

Available states:
- initial, boosters and tank attached
- prepare, leg rests rotate down, leapfrog is attached to spring which is draged into main tank, (leapfrog goes to deep space mode)
- launch sequence - Stoppable countdown, smoke and small engine fire, 
- launch execute - Unstoppable countdown, booster fire, is released and all goes up
*/

class LaunchSite : public CompoundObject
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1
	};

private:
   bool m_leftFootContact;
   bool m_rightFootContact;



public:
	LaunchSite(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      std::string& defXmlFileName);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void leapfrogFootTouch(b2Contact* contact, bool leftFoot);
   void leapfrogFootLift(b2Contact* contact, bool leftFoot);

};

