
#pragma once


class SteeringManager
{
private:
	b2Body* m_hostBody;
	b2Vect m_steering;
   
   // Variable for specific behaviours
   
   // Wander variables
   float m_wanderAngle;

	
	SteeringManager(b2Body* host);
	
	b2Vect doSeek(b2Vect target, float slowingRadius = 0.0f);
	b2Vect doFlee(b2Vect target);
	b2Vect doWander();
	b2Vect doEvade(b2Body* target);
	b2Vect doPursuit(b2Body* target);
	
public:
	void update( void );
	
	void reset( void );
	
	void seek(b2Vect target, float slowingRadius = 0.0f);
	void flee(b2Vect target);
	void wander();
	void evade(b2Body* target);
	void pursuit(b2Body* target);
}
