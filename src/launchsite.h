#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"

DECLARE_SMART(LaunchSite, spLaunchSite);

class LaunchSite : public CompoundObject, public CollisionEntity
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1
	};

public:
	LaunchSite(
      oxygine::Resources& gameResources,
      oxygine::Actor* parent,
      b2World* world,
      const oxygine::Vector2& pos,
      std::string& defXmlFileName);

   virtual CollisionEntityTypeEnum getEntityType(void);
};

