#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"

DECLARE_SMART(LandingActor, spLandingActor);

class LandingActor : public SceneActor
{
private:
   void modeReachedListener(oxygine::Event *ev);
   void handlePropertyTriggeredEvent(oxygine::Event *ev);

public:
	LandingActor(oxygine::Resources& gameResources, std::string& fileName, std::string& initialState);

};
