#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "planetactor.h"

DECLARE_SMART(ReentryCompositeActor, spReentryCompositeActor);

class ReentryCompositeActor : public Actor
{
private:
   spPlanetActor m_planetActor;

public:
   ReentryCompositeActor(oxygine::Resources& gameResources);

};
