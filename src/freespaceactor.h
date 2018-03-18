#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"

DECLARE_SMART(FreeSpaceActor, spFreeSpaceActor);

class FreeSpaceActor : public SceneActor
{
public:
	FreeSpaceActor(oxygine::Resources& gameResources);

};
