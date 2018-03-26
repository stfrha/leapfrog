#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(FlameEmitter, spFlameEmitter);

class FlameEmitter : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResources;

   bool m_emit;

	b2Body * m_emitterBody;

   // The focus origo of the emitter and the center of the emitter line.
   // Expressed in local coordinates of the body (i.e. related to its focus point)
   b2Vec2 m_emitterOrigin;
	
	// The angle in relation to the emitter body defines direction for particle to flow
	float m_emittAngle;		

	// Particles are generate from a line with this width perpendicular to the emittAngle
	float m_emitterWidth;

	// Number of particle per second
	float m_intensity;

	int m_lifetime; // [ms}

   float m_impulseMagnitude;

	float m_noiseConeAngle;	// Defines a cone angle of noise added to direction

   float m_radius;


public:
	FlameEmitter(
      oxygine::Resources& gameResources, 
      b2Body* body, 
      b2Vec2 emitterOrigin, 
      float angle, 
      float emitterWidth,
      float intensity,
      float lifetime,
      float impulseMagnitude,
      float radius);
	void startEmitter(void);
	void stopEmitter(void);
   void setParameters(float intensity,
      float lifetime,
      float impulseMagnitude,
      float radius);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
