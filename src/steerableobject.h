#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "steeringmanager.h"
#include "flameemitter.h"
#include "gun.h"


class SceneActor;
class AsteroidField;

DECLARE_SMART(SteerableObject, spSteerableObject);

class SteerableObject : public CompoundObject
{
public:
   enum launchState
   {
      start,
      goRight,
      goUp,
      goLeft,
      goDown
   };


private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   b2Body* m_body;

   launchState m_state;
   bool m_slowTurningAfterHit;
   oxygine::timeMS m_stateStartTime;
   SteeringManager* m_steeringManager;
   b2Vec2 m_seekTarget;

   int   m_damage;

   SceneActor* m_sceneActor;

   //oxygine::spSprite m_aheadCircle;
   //oxygine::spSprite m_ahead2Circle;


   
public:
   spFlameEmitter m_boosterFlame;
   spGun m_gun;

	SteerableObject(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root);




   void hitByAnything(b2Contact* contact);
   void hitByBullet(b2Contact* contact); 
   void hitShield(b2Contact* contact);     // Returns true if SteerableObject was shattered
   void hitByLepfrog(b2Contact* contact);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
