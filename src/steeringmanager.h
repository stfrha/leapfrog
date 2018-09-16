#pragma once

#include "Box2D/Box2D.h"
#include "steeringbody.h"
#include "collisionentity.h"

class SceneActor;

class SteeringManager
{

private:
	SteeringBody* m_hostBody;
	b2Vec2 m_steering;
   SceneActor* m_sceneActor;

   b2Vec2 m_lastKnowTargetPos;
   bool m_targetIsHiding;
   bool m_hunterIsWandering;
   float m_maxVelocity;

   b2Vec2 doSeek(b2Vec2 target, float maxVelocity, float slowingRadius = 0.0f, float turnBooster = 1.0f);
	b2Vec2 doFlee(b2Vec2 target, float maxVelocity);
	b2Vec2 doWander(float maxVelocity);
   b2Vec2 doEvade(b2Body* target, float maxVelocity);
	b2Vec2 doPursuit(b2Body* target, float maxVelocity);
   b2Vec2 doStayInScene(void);
   b2Vec2 doAvoidCollision(void);
   b2Body* findMostThreatening(b2Vec2 pos, b2Vec2 ahead, b2Vec2 ahead2);

   b2Vec2 doWanderHunt(b2Body* target, float maxVelocity);
	
public:
   float m_wanderAngle;

   b2Vec2 m_debugAhead;
   b2Vec2 m_debugAhead2;
   float m_debugRadius;

   SteeringManager(SteeringBody* host, SceneActor* sceneActor);

   void update( void );
	
	void reset( void );
	
	void seek(b2Vec2 target, float maxVelocity, float slowingRadius = 0.0f, float turnBooster = 1.0f);
	void flee(b2Vec2 target, float maxVelocity);
	void wander(float maxVelocity);
   void evade(b2Body* target, float maxVelocity);
	void pursuit(b2Body* target, float maxVelocity);
   void wanderHunt(b2Body* target, float maxVelocity);

   b2Vec2 getSteering();

};
