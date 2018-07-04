#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"

DECLARE_SMART(LandingPad, spLandingPad);

/*
Available events:
- leapfrog landed  
*/

class LandingPad : public CompoundObject
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
	LandingPad(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void leapfrogFootTouch(b2Contact* contact, bool leftFoot);
   void leapfrogFootLift(b2Contact* contact, bool leftFoot);

};

