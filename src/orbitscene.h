#pragma once
#include "oxygine-framework.h"
#include "Box2d/Box2D.h"
#include "compoundobject.h"
#include "orbitsceneevents.h"
#include "gamestatus.h"

class OrbitSpaceScene;
class PlanetActor;
class LeapFrog;


DECLARE_SMART(OrbitScene, spOrbitScene);

class OrbitScene : public CompoundObject
{
public:
   enum States
   {
      enteringOrbit,       // Leapfrog is accelerating to orbit, background is moving 
      waitForUserBreak,    // Monitor booster key to enter break
      breaking,            // User is breaking
      reentry,             // Break is complete and reentry is executing
      touchDown            // Reentry is complete
   };

   OrbitSpaceScene * m_space;

private:
   PlanetActor* m_planet;
   LeapFrog* m_leapfrog;
   States m_state;
   int m_stateChangeTime;
   
   //  FreeSpaceContactListener   m_contactListener;

   void orbitEstablishedHandler(oxygine::Event *ev);


public:
	OrbitScene(
      oxygine::Resources& gameResources,
      spGameStatus gameStatus,
      const std::string& fileName,
      const std::string& initialState);

protected:
   void doUpdate(const oxygine::UpdateState &us);

};
