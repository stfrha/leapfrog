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


   void collisionBlast(b2Contact* contact, bool small = true);
   void evaluateDamage(void);
   
public:
   spFlameEmitter m_boosterFlame;
   spGun m_gun;

	SteerableObject(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

   void hitImpulse(b2Contact* contact, const b2ContactImpulse* impulse);
   void hitByBullet(b2Contact* contact, float bulletEqvDamage);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
