#pragma once

#include <set>

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"

class LandingSite
{
public:
   std::string m_name;
   float m_angle;
   std::string m_texture;

public:
   LandingSite() :
      m_name("noName"),
      m_angle(0.0f),
      m_texture("noTexture")
   {  }

   LandingSite(std::string name, float angle, std::string texture) :
      m_name(name),
      m_angle(angle),
      m_texture(texture)
   {  }

   LandingSite(pugi::xml_node& objectNode) 
   {  
      m_name = objectNode.attribute("name").as_string();
      m_angle = objectNode.attribute("angle").as_float();
      m_texture = objectNode.attribute("texture").as_string();
   }

   bool operator< (const LandingSite & obj) const
   {
      return (this->m_angle < obj.m_angle);
   }
};

class LandingResult
{
public:
   std::string m_closestSite;
   float m_angleFromClosestSite;
   bool m_landingSuccessfull;
   LandingResult() :
      m_closestSite(""),
      m_angleFromClosestSite(0.0f),
      m_landingSuccessfull(false)
   { }
};



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

   // Leapfrog is burning its booster to break
   PAS_BURN,

   // Leapfrog has done break burn and starts to move toward planet
   PAS_REENTRY,

   // Leapfrog has reached the landing site
   PAS_LANDED
};

class SceneActor;

class PlanetActor : public CompoundObject
{
private:
   const static float c_relativeTrajectoryAngles[];
   const static float c_trajectoryAngles[];
   const static oxygine::Vector2 c_trajectoryPositions[];
   const static oxygine::Vector2 c_bouncyTrajectoryPositions[];
   const float c_predictedAngleOffset;
   const float c_angleZeroRotation;
   const float c_landingPointAngle;
   const float c_startOffset;
   const float c_safeLandingMaxRadDeviation;
   


   std::set<LandingSite> m_landingSites;

   PlanetActorStateEnum m_state;
   bool m_stateChange;
   oxygine::Vector2 m_orbitStartPos;
   LandingResult m_results;


   float m_planetAngleAtOrbitEstablished;
   float m_planetAngleAtBurnStart;
   float m_planetAngleAtSurface;


   oxygine::Resources* m_gameResources;
   oxygine::spActor m_planet;
   oxygine::spSprite m_positionIndicator;
   oxygine::spProgressBar m_burnIndicator;
   oxygine::spTweenQueue m_frogTweenQueue;



public:
   PlanetActor(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      const pugi::xml_node& objectNode);
   void orbitEstablished(void);
   void startReentry(float relativeBurnDeviation, float deviationDistance);
   void surfaceReached(void);
   void startBurn(void);
   void stopBurn(void);
   float getBurnAmount(void);
   float getReentryLeapfrogAngle(float time);
   LandingResult getLandingResult();

protected:
   void doUpdate(const oxygine::UpdateState& us);

};
