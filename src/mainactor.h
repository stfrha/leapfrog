#pragma once

#include "landingactor.h"
#include "oxygine-framework.h"

//DECLARE_SMART is helper, it does forward declaration and declares intrusive_ptr typedef for your class
DECLARE_SMART(MainActor, spMainActor)

class MainActor : public oxygine::Actor
{
private:
	spLandingActor m_landingActor;

	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
	oxygine::Resources m_gameResources;

public:

	MainActor();
	~MainActor();

	void onSystemEvent(Event* ev);

	void doUpdate(const UpdateState& us);
};

