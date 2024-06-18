#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"


DECLARE_SMART(OrbitSpaceScene, spOrbitSpaceScene);

class OrbitSpaceScene : public SceneActor
{
public:
   enum States
   {
      waitForLeapfrog,     // Leapfrog has not yet been added to the scene
      enteringOrbit,       // Leapfrog is accelerating to orbit, background is moving 
      slowToStop,          // background is slowing down and leapfrog also using the damping
      turnBooster,         // Fire steering booster and start to turn leapfrog
      goToModeReentry,     // While turning, go to mode Reentry
      orbitEstablished,    // Show trajectory and generate landing sites, wait for break burn

   };

private:
   b2Body* m_leapfrogBody;

   States m_state;
   oxygine::timeMS m_stateStartTime;

//   FreeSpaceContactListener   m_contactListener;


   void atOrbitEstablished(Event* event);

public:
	OrbitSpaceScene(
      b2World* world,
      pugi::xml_node& root,
      int groupIndex);

   void startLeapfrogInScene(std::string name);

protected:
   void doUpdate(const oxygine::UpdateState &us) override;

};
