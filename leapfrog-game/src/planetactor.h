#pragma once

#include <set>

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"

// A landing site on the planet (orbit_scene.xml, <landingSite>): name,
// angle in degrees, the marker's texture, the landing scene it leads to
// (scene, e.g. "landing_scene.xml"; "" = the scripts decide) and the label
// shown under its marker in orbit (label; the name if not given).
class LandingSite
{
public:
   std::string m_name;
   float m_angle;
   std::string m_texture;
   std::string m_scene;
   std::string m_label;

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
      m_angle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
      m_texture = objectNode.attribute("texture").as_string();
      m_scene = objectNode.attribute("scene").as_string();
      m_label = objectNode.attribute("label").as_string(m_name.c_str());
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
   int m_closestSiteIndex;          // in angle order (as the sites are kept)
   std::string m_closestSiteScene;
   float m_angleFromClosestSite;
   float m_distanceFromClosestSite; // along the surface, planet units (a
                                    // site's marker is 100 wide)
   bool m_landingSuccessfull;       // on the site's marker
   LandingResult() :
      m_closestSite(""),
      m_closestSiteIndex(-1),
      m_angleFromClosestSite(0.0f),
      m_distanceFromClosestSite(0.0f),
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
   const float c_startAngleOffset;
   const float c_safeLandingMaxRadDeviation;
   static const float c_orbitSpinPerMs;
   static const float c_spinUpMs;
   static const float c_arrivalStartX;

   float m_spinElapsedMs;     // since the scene started (the spin-up)

   // The predicted ("solution") markers' angles on the planet, set when
   // the orbit is established. A marker is at the solution (a burn now
   // lands on its site) when the planet's turn brings it to
   // c_landingPointAngle.
   std::vector<float> m_predictedSiteAngles;
   std::vector<oxygine::spActor> m_siteMarkers;
   void placeSiteMarkers(bool fadeIn);
   static const float c_missedAngle;

   std::set<LandingSite> m_landingSites;

   PlanetActorStateEnum m_state;
   bool m_stateChange;
   oxygine::Vector2 m_orbitStartPos;
   LandingResult m_results;
   CompoundObject* m_parentObject;

   float m_planetAngleAtOrbitEstablished;
   float m_planetAngleAtBurnStart;
   float m_planetAngleAtSurface;


   oxygine::spActor m_planet;
   oxygine::spSprite m_positionIndicator;
   oxygine::spProgressBar m_burnIndicator;
   oxygine::spTweenQueue m_frogTweenQueue;



public:
   std::string m_spaceSceneBackground;
   std::string m_spaceSceneFile;

   PlanetActor(
      CompoundObject* parentObject,
      const pugi::xml_node& objectNode);
   void orbitEstablished(void);
   void startReentry(float relativeBurnDeviation, float deviationDistance);
   void surfaceReached(void);
   void startBurn(void);
   void stopBurn(void);
   float getBurnAmount(void);
   float getReentryLeapfrogAngle(float time);

   // True when every site's predicted marker has passed its solution by
   // c_missedAngle without a burn (a burn now would come down far off
   // every site): the markers come round again (OrbitScene,
   // restartRevolution). False with no sites, or before the orbit is
   // established.
   bool allSolutionsPassed(void);

   // The next revolution: the site markers come again (a missed orbit)
   void restartRevolution(void);
   LandingResult getLandingResult();

protected:
   void doUpdate(const oxygine::UpdateState& us) override;

};
