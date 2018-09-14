#pragma once

#include "Box2D/Box2D.h"
#include "steeringbody.h"

class SceneActor;

class SteeringManager
{
private:
	SteeringBody* m_hostBody;
	b2Vec2 m_steering;
   SceneActor* m_sceneActor;

   b2Vec2 doSeek(b2Vec2 target, float slowingRadius = 0.0f);
	b2Vec2 doFlee(b2Vec2 target);
	b2Vec2 doWander();
   b2Vec2 doSeekWander();
   b2Vec2 doEvade(b2Body* target);
	b2Vec2 doPursuit(b2Body* target);
   b2Vec2 doStayInScene(void);
   b2Vec2 doAvoidCollision(void);
   b2Body* findMostThreatening(b2Vec2 pos, b2Vec2 ahead, b2Vec2 ahead2);

	
public:
   float m_wanderAngle;

   b2Vec2 m_debugAhead;
   b2Vec2 m_debugAhead2;
   float m_debugRadius;

   SteeringManager(SteeringBody* host, SceneActor* sceneActor);

   void update( void );
	
	void reset( void );
	
	void seek(b2Vec2 target, float slowingRadius = 0.0f);
	void flee(b2Vec2 target);
	void wander();
   void seekWander();
   void evade(b2Body* target);
	void pursuit(b2Body* target);

   b2Vec2 getSteering();

};
