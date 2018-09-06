#pragma once

#include "Box2D/Box2D.h"
#include "steeringinterface.h"

class SteeringBody : public b2Body, ISteering
{
private:
   const float c_maxVelocity;
   
   b2Body(const b2BodyDef* bd, b2World* world);
   ~b2Body();


   // TODO: Rethink this!!! The SteeringBody class probably needs the b2Body as a
   // pointer member since the constructor is private and a body normally is 
   // created using b2World->CreateBody(def);




	SteeringBody(
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
