#pragma once

#include "oxygine-framework.h"
#include "collisionentity.h"

DECLARE_SMART(BodyUserData, spBodyUserData);

class BodyUserData
{
public:
   CollisionEntityTypeEnum m_collisionType;
   oxygine::spActor m_actor;
};

