#pragma once

#include "oxygine-framework.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "reentrysceneactor.h"

enum SceneTypeEnum
{
   STE_LANDING,
   STE_FREE_SPACE,
   STE_ORBIT
};

//DECLARE_SMART is helper, it does forward declaration and declares intrusive_ptr typedef for your class
DECLARE_SMART(MainActor, spMainActor)

class MainActor : public oxygine::Actor
{
private:
	//spLandingActor m_landingActor;
 //  spFreeSpaceActor m_freeSpaceActor;
 //  spPlanetActor m_planetActor;

	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
	oxygine::Resources m_gameResources;

   spBox2DDraw m_debugDraw;


public:

	MainActor();
	~MainActor();

   void changeToMode(SceneTypeEnum scene);

	void onSystemEvent(Event* ev);

	void doUpdate(const UpdateState& us);
};

