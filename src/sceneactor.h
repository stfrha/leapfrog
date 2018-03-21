#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "leapfrog.h"
#include "scales.h"
#include "physdispconvert.h"
#include "actortodie.h"

DECLARE_SMART(SceneActor, spSceneActor);

class SceneActor : public oxygine::Actor
{
protected:
   float	m_zoomScale;
   float	m_stageToViewPortScale;
   float	m_physToStageScale;
   b2World * m_world;
   spLeapFrog  m_leapfrog;
   std::vector<ActorToDie*> m_deathList;


public:
	SceneActor(oxygine::Resources& gameResources);
   ~SceneActor();

   b2World* GetWorld(void);
   void addMeToDeathList(ActorToDie* actor);
   void addBlast();

protected:
	void doUpdate(const UpdateState& us);

   void sweepKillList(void);

   void createLeapFrog(oxygine::Resources& gameResources);
};
