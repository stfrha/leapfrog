#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"


DECLARE_SMART(SceneActor, spSceneActor);

class SceneActor : public CompoundObject
{
public:
   enum PanorateModeEnum
   {
      top,
      center,
      bottom,
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
   bool m_externalControl;
   SceneTypeEnum m_sceneType;

public:
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
      const std::string& initialState,
      int groupIndex);

   float m_sceneWidth;
   float m_sceneHeight;

   bool m_turnLeftPressed;
   bool m_turnRightPressed;
   bool m_boosterPressed;
   bool m_firePressed;
   bool m_zoomInPressed;
   bool m_zoomOutPressed;
   spLeapFrog  m_leapfrog;
   HeadDownDisplay* m_map;

   b2World* GetWorld(void);
   Resources* getResources(void);

   void setPanorateMode(PanorateModeEnum mode);
   void setPanorateObject(CompoundObject* co);
   void setZoom(float zoom);

   void addMeToDeathList(spActor actor);
   void addObjectToSpawnList(
      int numOfSpawns,
      oxygine::Vector2 leftTop,
      oxygine::Vector2 widthHeight,
      spSpawnObjectList spawnSource);
   
   void registerObjectsToMap(void);

  void takeControlOfLeapfrog(bool control);

  SceneActor::SceneTypeEnum getSceneType(void);

protected:
	void doUpdate(const UpdateState& us);

   void sweepKillList(void);
   void sweepSpawnList(void);

};
