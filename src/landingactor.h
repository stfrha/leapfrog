#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"

DECLARE_SMART(LandingActor, spLandingActor);

class LandingActor : public SceneActor
{
public:
	LandingActor(oxygine::Resources& gameResources, std::string fileName);

};
