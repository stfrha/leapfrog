#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "planetactor.h"

DECLARE_SMART(ReentrySceneActor, spReentrySceneActor);

class ReentrySceneActor : public SceneActor
{
private:
   spPlanetActor m_planetActor;

public:
   ReentrySceneActor(oxygine::Resources& gameResources);

};
