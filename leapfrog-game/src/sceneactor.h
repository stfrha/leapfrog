#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"
#include "softboundary.h"
#include "parallaxbackground.h"

class ManualPan
{
public:
   ManualPan();

   oxygine::Vector2  m_manualPanPos;
   bool m_manPanEnable;
};

class SceneTimer
{
public:
   int m_id;
   int m_timeOutNumOfTicks;
   int m_ticks;

public:
   SceneTimer(int id, int timeOutNumOfTicks);
   bool tickTimer(void);
};



DECLARE_SMART(SceneActor, spSceneActor);

class SceneActor : public CompoundObject
{
public:
   enum PanorateModeEnum
   {
      top,
      midTop,
      center,
      bottom,
      midBottom,
      topLeft,
      fix
   };

   enum SceneTypeEnum
   {
      landing,
      deepSpace,
      orbit,
      hyperSpace
   };

   enum LaunchStateEnum
   {
      fadeIn,
      sceneIsOn,
      fadeOut
   };

private:
   float m_secondsPerUpdate = 1.0f / 60.0f;
   spCompoundObject m_panObject;
   bool m_armManPanEnableChange;
   bool m_armPauseChange;
   std::vector<spSoftBoundary> m_boundaries;
   std::vector<b2Body*> m_boundedBodies;
   ManualPan* m_manualPan;
   std::vector<SceneTimer> m_timers;
   int m_timerIdCounter;

   std::vector<ParallaxBackground> m_parallaxBackgrounds;

   // When true, each parallax layer gets one uniform scale (the larger of
   // the x and y scales) so sprites on it keep their aspect ratio. The
   // layer's bottom edge stays where the per-axis scaling put it and the
   // extra size is split evenly left and right. Set from the scene's
   // sceneProperties attribute parallaxKeepAspect (default true).
   bool m_parallaxKeepAspect = true;

   // wantedVpPos is the position in main actor coordinates where we
   // want to keep the panorating body. The whole scene is moved to
   // make the body at the specified position.
   // This can be animated for smooth transitions.
   oxygine::Vector2 m_wantedVpPos;

   // This can be animated for smooth transitions.
   float m_zoomPos;

protected:
   float	m_zoomScale;
   float	m_defaultZoomScale;   // where the zoom-out at scene start ends
   float	m_stageToViewPortScale;
   float	m_physToStageScale;
   b2World * m_world;
   std::vector<oxygine::spActor> m_deathList;
   std::vector<spSpawnInstruction> m_spawnInstructions;
   PanorateModeEnum m_panorateMode;
   bool m_panorateLimitEnabled;
   bool m_externalControl;
   SceneTypeEnum m_sceneType;

public:
   const Vector2& getWantedVpPos() const; 
   void setWantedVpPos(const Vector2& pos);
   void setManualPan(ManualPan* mp);

   const float& getZoom() const;
   void setZoom(const float& pos);

   // Zoom smoothly so that `width` game units are visible across the
   // screen, e.g. when the Leapfrog has landed on a pad. Only zooms in:
   // if the view is already closer than that, it is left alone.
   void zoomInToVisibleWidth(float width, oxygine::timeMS duration);

   // Zoom smoothly back out to the scene's default zoom, the one the
   // zoom-out at scene start ends at. Only zooms out.
   void zoomOutToDefault(oxygine::timeMS duration);

	SceneActor(
      b2World* world, 
      float initialZoomScale,
      float zoomScale);
   ~SceneActor();

   static SceneActor* defineScene(
      CompoundObject* parentObject,
      b2World* world,
      pugi::xml_node& root,
      int groupIndex);

   //float m_sceneWidth;
   //float m_sceneHeight;

   bool m_turnLeftPressed;
   bool m_turnRightPressed;
   bool m_boosterPressed;
   bool m_firePressed;
   bool m_pausePressed;
   bool m_zoomInPressed;
   bool m_zoomOutPressed;
   bool m_panButtonPressed;
   bool m_manPanEnablePressed;
   oxygine::Vector2 m_panVector;
   LeapFrog* m_leapfrog;

   b2World* getWorld(void);

   void addParallaxBackground(ParallaxBackground* background);
   void addParallaxBackgroundSprite(oxygine::spSprite sp, float parallaxAmount);

   void addBoundingBody(b2Body* body);
   void removeBoundingBody(b2Body* body);

   // Destroys a body and takes it off the bounded-bodies list first. Use
   // this instead of b2World::DestroyBody for bodies that may be bounded:
   // a destroyed body left on the list kept getting the boundary force
   // through the stale pointer, which Box2D reuses for the next body it
   // creates (often a flame particle, shot back into the scene).
   void destroyBody(b2Body* body);
   void testForBoundaryRepel(void);
   int isInsideTransitField(b2Body* body);

   void setPanorateMode(PanorateModeEnum mode);
   void setPanorateObject(CompoundObject* co);
   void enablePanorateLimit(bool enable);

   void addMeToDeathList(spActor actor);
   void addObjectToSpawnList(
      int numOfSpawns,
      oxygine::Vector2 leftTop,
      oxygine::Vector2 widthHeight,
      spSpawnObjectList spawnSource);
   
   int createSceneTimer(int numOfTicks);

   virtual void startLeapfrogInScene(std::string name);
   virtual void startLaunchSiteInScene(std::string name);

   void takeControlOfLeapfrog(bool control);

   SceneActor::SceneTypeEnum getSceneType(void);

   typedef Property2Args<float, oxygine::Vector2, const oxygine::Vector2&, SceneActor, &SceneActor::getWantedVpPos, &SceneActor::setWantedVpPos>  TweenWantedVpPos;
   typedef Property2Args<float, float, const float&, SceneActor, &SceneActor::getZoom, &SceneActor::setZoom>  TweenZoom;



protected:
	void doUpdate(const UpdateState& us) override;

   void sweepKillList(void);
   void sweepSpawnList(void);
   void removeTimer(int id);

};
