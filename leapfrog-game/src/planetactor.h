#pragma once

#include <set>

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"

// A landing site on the planet (<landingSite> in the planet's surface file,
// or in the orbit scene's properties without one): name, angle in degrees
// along the surface (growing to the right, clockwise, as on the editor's
// ruler: the surface file's x / its radius), the marker's texture ("" = no
// marker, e.g. when the surface's art shows the site), the landing scene
// it leads to (scene, e.g. "landing_scene.xml"; "" = the scripts decide)
// and the label under its solution marker in orbit (label; the name if
// not given). Sites are numbered in file order (the orbit scene's exit
// parameter).
//
// hidden="true": an unknown site. It has no marker and no solution marker,
// isn't a place to come down at (re-entry measures to the shown sites
// only) and doesn't hold up the next revolution, until Lua shows it with
// c_setLandingSiteShown(name, true), usually in the orbit scene's setup
// by the mission state. The surface's art is not hidden.
class LandingSite
{
public:
   std::string m_name;
   float m_angle;
   std::string m_texture;
   std::string m_scene;
   std::string m_label;
   bool m_hidden;    // hidden="true" in the file
   bool m_shown;     // now (starts as !m_hidden; Lua can change it)

public:
   LandingSite() :
      m_name("noName"),
      m_angle(0.0f),
      m_texture("noTexture"),
      m_hidden(false),
      m_shown(true)
   {  }

   LandingSite(std::string name, float angle, std::string texture) :
      m_name(name),
      m_angle(angle),
      m_texture(texture),
      m_hidden(false),
      m_shown(true)
   {  }

   LandingSite(pugi::xml_node& objectNode) 
   {  
      m_name = objectNode.attribute("name").as_string();
      m_angle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
      m_texture = objectNode.attribute("texture").as_string();
      m_scene = objectNode.attribute("scene").as_string();
      m_label = objectNode.attribute("label").as_string(m_name.c_str());
      m_hidden = objectNode.attribute("hidden").as_bool(false);
      m_shown = !m_hidden;
   }

};

class LandingResult
{
public:
   std::string m_closestSite;
   int m_closestSiteIndex;          // in file order
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

   // The planet's surface: a layer turning with the planet that holds the
   // surface's art (spriteBox elements of the surface file, placed from
   // the unrolled strip: x along the surface, y the height, negative
   // above the ground), the sites' markers, and from the orbit's
   // establishment the sites' solution markers (predicted marker and
   // label, c_predictedAngleOffset ahead of each site: a burn when it is
   // at c_landingPointAngle lands on the site). In the layer, angle a
   // along the surface is at local angle a; the layer's rotation puts
   // the sites where the re-entry timing expects them when the orbit is
   // established (c_startAngleOffset), and puts them back there for each
   // new revolution (restartRevolution).
   oxygine::spActor m_surface;
   std::vector<oxygine::spActor> m_solutionMarkers;
   bool m_revolutionPending;  // fading out for the next revolution
   void loadSurface(const pugi::xml_node& orbitProperties, std::string& texture, float& tileWidth, float& tileHeight);
   void placeSurfaceSprite(pugi::xml_node& node);
   void placeSurfacePolygon(pugi::xml_node& node);
   oxygine::Vector2 bendToSurface(const oxygine::Vector2& stripPoint);
   void placeSolutionMarkers(bool fadeIn);
   float surfaceRotationFor(float planetRotation);
   static const float c_missedAngle;
   static const float c_surfaceRadius;

   std::vector<LandingSite> m_landingSites;
   std::vector<oxygine::spActor> m_siteMarkers;   // per site (NULL: no marker texture)

   // The planet of the orbit scene being shown (for Lua), NULL if none
   static PlanetActor* s_current;

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
   ~PlanetActor();

   // The planet of the orbit scene being shown, NULL outside the orbit
   // scene
   static PlanetActor* current(void);
   static void clearCurrent(void);

   // Shows or hides a landing site by name (see LandingSite's hidden).
   // Takes effect at once: its marker, and its solution marker if the
   // orbit is established and no burn has started. False if the planet
   // has no site by that name.
   bool setSiteShown(const std::string& name, bool shown);
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
