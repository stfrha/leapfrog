#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "scales.h"
#include "physdispconvert.h"

enum PanorateModeEnum
{
   PME_TOP,
   PME_CENTER,
   PME_BOTTOM,
   PME_TOP_LEFT,
   PME_FIX
};

enum SceneTypeEnum
{
   STE_LANDING,
   STE_FREE_SPACE,
   STE_ORBIT
};

DECLARE_SMART(SceneActor, spSceneActor);

class SceneActor : public CompoundObject
{
public:
   enum launchState
   {
      fadeIn,
      sceneIsOn,
      fadeOut
   };

protected:
   oxygine::Resources * m_gameResources;
   float	m_zoomScale;
   float	m_stageToViewPortScale;
   float	m_physToStageScale;
   b2World * m_world;
   spLeapFrog  m_leapfrog;
   spLaunchSite m_launchSite;
   std::vector<CompoundObject*> m_deathList;
   PanorateModeEnum m_panorateMode;
   bool m_externalControl;
   SceneTypeEnum m_sceneType;

public:
	SceneActor(oxygine::Resources& gameResources, float zoomScale);
   ~SceneActor();

   b2World* GetWorld(void);
   Resources* getResources(void);

   void setLeapfrog(spLeapFrog lf);
   void setPanorateMode(PanorateModeEnum mode);

   void addMeToDeathList(CompoundObject* actor);

   void takeControlOfLeapfrog(bool control);

   SceneTypeEnum getSceneType(void);

protected:
	void doUpdate(const UpdateState& us);

   void sweepKillList(void);

   void createLeapFrog(oxygine::Resources& gameResources);
};
