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
   spCompoundObject m_panObject;
   bool m_armManPanEnableChange;
   std::vector<spSoftBoundary>   m_boundaries;
   std::vector<b2Body*>   m_boundedBodies;
   ManualPan* m_manualPan;
   std::vector<SceneTimer> m_timers;
   int m_timerIdCounter;

   std::vector<ParallaxBackground> m_parallaxBackgrounds;

   // wantedVpPos is the position in main actor coordinates where we
   // want to keep the panorating body. The whole scene is moved to
   // make the body at the specified position.
   // This can be animated for smooth transitions.
   oxygine::Vector2 m_wantedVpPos;

protected:
   oxygine::Resources * m_gameResources;
   float	m_zoomScale;
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


	SceneActor(
      oxygine::Resources& gameResources, 
      b2World* world, 
      float zoomScale);
   ~SceneActor();

   static SceneActor* defineScene(
      Resources& gameResources,
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
   bool m_zoomInPressed;
   bool m_zoomOutPressed;
   bool m_panButtonPressed;
   bool m_manPanEnablePressed;
   oxygine::Vector2 m_panVector;
   spLeapFrog  m_leapfrog;
   HeadDownDisplay* m_map;

   b2World* getWorld(void);
   Resources* getResources(void);

   void addParallaxBackground(ParallaxBackground* background);
   void addParallaxBackgroundSprite(oxygine::spSprite sp, float parallaxAmount);

   void addBoundingBody(b2Body* body);
   void removeBoundingBody(b2Body* body);
   void testForBoundaryRepel(void);
   bool isInsideOrbitField(b2Body* body);

   void setPanorateMode(PanorateModeEnum mode);
   void setPanorateObject(CompoundObject* co);
   void setZoom(float zoom);
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


protected:
	void doUpdate(const UpdateState& us);

   void sweepKillList(void);
   void sweepSpawnList(void);
   void removeTimer(int id);

};
