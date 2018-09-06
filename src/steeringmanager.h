#pragma once

#include "Box2D/Box2D.h"


class SteeringManager
{
private:
	b2Body* m_hostBody;
	b2Vec2 m_steering;
   
   // Variable for specific behaviours
   
   // Wander variables
   float m_wanderAngle;


   // Steering properties used to define the behaviour
   const float c_maxVelocity;
   const float c_wanderCircleDistance;
   const float c_wanderCircleRadius;
   const float c_wanderAngleChange;
   
	
	SteeringManager(
      b2Body* host,
      float maxVelocity,
      float wanderCircleDistance,
      float wanderCircleRadius,
      float wanderAngleChange
   );
	
	b2Vec2 doSeek(b2Vec2 target, float slowingRadius = 0.0f);
	b2Vec2 doFlee(b2Vec2 target);
	b2Vec2 doWander();
	b2Vec2 doEvade(b2Body* target);
	b2Vec2 doPursuit(b2Body* target);
	
public:
	void update( void );
	
	void reset( void );
	
	void seek(b2Vec2 target, float slowingRadius = 0.0f);
	void flee(b2Vec2 target);
	void wander();
	void evade(b2Body* target);
	void pursuit(b2Body* target);
}
