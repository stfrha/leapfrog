#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"

DECLARE_SMART(PlanetActor, spPlanetActor);

enum PlanetActorStateEnum
{
   // Spinning planet, no landing sites
   PAS_INITITAL,                 

   // Leapfrog has done orbit burn and turned to 
   // brake position
   // Landing sites are spawned and placed on planet
   // Trajectory is displayed
   PAS_ORBIT_ESTABLISHED,        

   // Leapfrog has done break burn and starts to move toward planet
   PAS_REENTRY,

   // Leapfrog has reached the landing site
   PAS_LANDED
};


class PlanetActor : public oxygine::Actor
{
private:
   const static float c_relativeTrajectoryAngles[];
   const static float c_trajectoryAngles[];
   const static oxygine::Vector2 c_trajectoryPositions[];

   oxygine::Resources* m_gameResources;
   PlanetActorStateEnum m_state;
   spActor m_planet;
   oxygine::Vector2 m_orbitStartPos;
   oxygine::spSprite m_positionIndicator;



public:
   PlanetActor(oxygine::Resources& gameResources);
   void orbitEstablished(void);
   void startReentry(void);
};
