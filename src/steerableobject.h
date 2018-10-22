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

   enum SteeringStateEnum
   {
      wander,
      seek,
      pursuit,
      flee,
      wanderHunt,
      fix
   };


private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   b2Body* m_body;
   bool m_slowTurningAfterHit;
   oxygine::timeMS m_stateStartTime;
   SteeringManager* m_steeringManager;

   SteeringStateEnum m_state;
   b2Vec2 m_seekPoint;
   b2Body* m_targetBody;
   float m_bodyToBoosterAngle;
   float m_maxBoosterForce;
   float m_maxSpeed;
   float m_maxRotateSpeed;

   SceneActor* m_sceneActor;


   void readSteerableObjectNode(pugi::xml_node root);
   void collisionBlast(b2Contact* contact, bool small = true);
   void evaluateDamage(void);
   void executeSteeringForce(b2Vec2 steeringForce);

   void firstTryForces(b2Vec2 steeringForce);
   void turnBoosterForce(b2Vec2 steeringForce);
   
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
