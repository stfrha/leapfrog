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

public:
	LandingPad(
      oxygine::Resources& gameResources,
      oxygine::Actor* parent,
      b2World* world,
      const oxygine::Vector2& pos,
      std::string& defXmlFileName);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void leapfrogLanded(b2Contact* contact);

};

