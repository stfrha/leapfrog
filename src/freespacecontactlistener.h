#pragma once

#include "Box2D/Box2D.h"
#include "sceneactor.h"

class FreeSpaceContactListener : public b2ContactListener
{
private:
   SceneActor * m_sceneActor;
public:
   void BeginContact(b2Contact* contact);
   void EndContact(b2Contact* contact);

   void setSceneActor(SceneActor* actor);
};
