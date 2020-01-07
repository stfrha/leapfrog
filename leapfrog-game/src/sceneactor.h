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


DECLARE_SMART(SceneActor, spSceneActor);

//class ParallaxBackground
//{
//public:
//   spSprite m_sprite;
//   float m_parallaxAmount;
//
//   ParallaxBackground(float parallaxAmount)
//   {
//      m_parallaxAmount = parallaxAmount;
//   }
//};
//
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
   std::vector<spSoftBoundary>   m_boundaries;
   std::vector<b2Body*>   m_boundedBodies;

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
   spLaunchSite m_launchSite;
   std::vector<oxygine::spActor> m_deathList;
   std::vector<spSpawnInstruction> m_spawnInstructions;
   PanorateModeEnum m_panorateMode;
   bool m_panorateLimitEnabled;
   bool m_externalControl;
   SceneTypeEnum m_sceneType;
   std::string m_initialState;

public:
   const Vector2& getWantedVpPos() const; 
   void setWantedVpPos(const Vector2& pos);

	SceneActor(
      oxygine::Resources& gameResources, 
      b2World* world, 
      float zoomScale,
      const std::string& initialState);
   ~SceneActor();

   static SceneActor* defineScene(
      Resources& gameResources,
      CompoundObject* parentObject,
      b2World* world,
      pugi::xml_node& root,
      const std::string& initialState,
      int groupIndex);

   //float m_sceneWidth;
   //float m_sceneHeight;

   bool m_turnLeftPressed;
   bool m_turnRightPressed;
   bool m_boosterPressed;
   bool m_firePressed;
   bool m_zoomInPressed;
   bool m_zoomOutPressed;
   spLeapFrog  m_leapfrog;
   HeadDownDisplay* m_map;

   b2World* getWorld(void);
   Resources* getResources(void);
   std::string* getInitialState(void);

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
   
   void registerObjectsToMap(void);

  void takeControlOfLeapfrog(bool control);

  SceneActor::SceneTypeEnum getSceneType(void);

  typedef Property2Args<float, oxygine::Vector2, const oxygine::Vector2&, SceneActor, &SceneActor::getWantedVpPos, &SceneActor::setWantedVpPos>  TweenWantedVpPos;


protected:
	void doUpdate(const UpdateState& us);

   void sweepKillList(void);
   void sweepSpawnList(void);

};
