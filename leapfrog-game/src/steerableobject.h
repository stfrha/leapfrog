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
#include "shield.h"

class SceneActor;
class AsteroidField;

DECLARE_SMART(SteerableObject, spSteerableObject);

class SteerableObject : public CompoundObject
{
public:

   enum SteerableObjectPropertyEnum
   {
      state,
      xPos,
      yPos,
      seekXPos,
      seekYPos,
      ammo,
      shield,
      fuel,
      credits,
      damage
   };

   enum LaunchStateEnum
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

   enum AimStateEnum
   {
      noAim,
      aim,
      shoot
   };

private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   b2Body* m_body;
   bool m_slowTurningAfterHit;
   oxygine::timeMS m_stateStartTime;
   SteeringManager* m_steeringManager;
   AimStateEnum m_aimState;
   oxygine::timeMS m_aimStateStartTime;
   int m_headDownDisplayItemId;
   bool m_armClockReset;


//   SteeringStateEnum m_state; now a property
   b2Vec2 m_seekPoint;
   b2Body* m_targetBody;
   std::string m_targetBodyStr;
   float m_bodyToBoosterAngle;
   float m_maxBoosterForce;
   float m_maxSpeed;
   float m_maxRotateSpeed;
   float m_linearDamping;

   float m_boosterScale;

   SceneActor* m_sceneActor;


   void readSteerableObjectNode(pugi::xml_node root);
   void collisionBlast(b2Contact* contact, bool small = true);
   void evaluateDamage(void);
   void executeSteeringForce(b2Vec2 steeringForce, const oxygine::UpdateState& us, bool isAvoiding);

   void firstTryForces(b2Vec2 steeringForce);
   void turnBoosterForce(b2Vec2 steeringForce);
   void directiveForce(b2Vec2 steeringVelChange, bool aim);

public:
   spFlameEmitter m_boosterFlame;
   spGun m_gun;
   spShield m_shield;

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

   void initObjectResources(oxygine::Actor* statusEventOriginator, spObjectResources resources = NULL) override;
   void connectToForeignObjects(void) override;

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
