#pragma once

#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "oxygine-framework.h"

class LanderContactListener : public b2ContactListener
{
private:
   SceneActor * m_sceneActor;

public:
   void InitContactListner(SceneActor* sceneActor);
   void BeginContact(b2Contact* contact);
   void EndContact(b2Contact* contact);
   void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
   void ContactHandler(b2Contact* contact, bool begin); // begin = false means the end of the contact
   //void KillDestroyable(b2Contact* contact, oxygine::Actor* actor);

};
