#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"

class SceneActor;

class SteeringManager
{
public:
   enum WanderHunterState
   {
      wanderState,
      seekState,
      pursuitState
   };

private:
	b2Body* m_hostBody;

	//b2Vec2 m_steering;
   SceneActor* m_sceneActor;

   b2Vec2 m_lastKnowTargetPos;
   bool m_targetIsHiding;
   oxygine::timeMS m_stateStartTime;

   // The m_desiredVelocity is the velocity that push the 
   // object closer to its target. Detours because of 
   // collision avoidance is not included. The desiredVelocity
   // must be cleared at the beginning of each update. It will be
   // added to by any steering operation that moves towards the 
   // target (wander, seek, pursuit...). It is used by the 
   // collision avoidance to determine if the path towards the target
   // is blocked
   b2Vec2 m_desiredVelocity;

   // 30 steps should be 0.5 s
   b2Vec2 m_velChangeFilter[30];

   b2Vec2 doSeek(b2Vec2 target, float maxVelocity, bool enableFire = false, float slowingRadius = 0.0f, float turnBooster = 1.0f);
	b2Vec2 doFlee(b2Vec2 target, float maxVelocity);
	b2Vec2 doWander(float maxVelocity);
   b2Vec2 doEvade(b2Body* target, float maxVelocity);
	b2Vec2 doPursuit(b2Body* target, float maxVelocity);
   b2Vec2 doStayInScene(void);
   b2Vec2 doAvoidCollision(float maxVelocity);
   b2Vec2 doAvoidCollision1(float maxVelocity);
   b2Vec2 doAvoidCollision2(float maxVelocity);
   bool doEvaluateGunFire(b2Body* target);
   b2Body* findMostThreatening(b2Vec2 pos, b2Vec2 ahead, b2Vec2 ahead2);
   b2Vec2 globalToLocalConversion(b2Vec2 ownGlobalPos, float cosAngle, float sinAngle, b2Vec2 targetGlobalPos);
   b2Vec2 localToGlobalConversion(b2Vec2 ownGlobalPos, float cosAngle, float sinAngle, b2Vec2 targetLocalPos);

   b2Vec2 doWanderHunt(const oxygine::UpdateState& us, b2Body* target, float maxVelocity);
	
public:
   float m_wanderAngle;
   WanderHunterState m_wanderHunterState;
   bool m_fireTrigger;

   b2Vec2 m_debugAhead;
   b2Vec2 m_debugAhead2;
   float m_debugRadius;

   SteeringManager(b2Body* host, SceneActor* sceneActor);

   //void update( void );
		
	b2Vec2 seek(b2Vec2 target, float maxVelocity, float slowingRadius = 0.0f, float turnBooster = 1.0f);
	b2Vec2 flee(b2Vec2 target, float maxVelocity);
	b2Vec2 wander(float maxVelocity);
   b2Vec2 evade(b2Body* target, float maxVelocity);
	b2Vec2 pursuit(b2Body* target, float maxVelocity);
   b2Vec2 wanderHunt(const oxygine::UpdateState& us, b2Body* target, float maxVelocity);
   b2Vec2 avoidCollision(float maxVelocity);
   bool evaluateGunFire(b2Body* target);
};
