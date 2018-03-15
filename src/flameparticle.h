#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(FlameParticle, spFlameParticle);

class FlameParticle : public oxygine::Actor
{
private:
	b2Body * m_emitterBody;
	
	// The angle in relation to the emitter body defines direction for particle to flow
	float m_emittAngle;		

	// Particles are generate from a line with this with perpendicular to the emittAngle
	float m_emitterWidth;

	// Number of particle per second
	float m_intensity;

	float m_lifetime; // [sec}

	float m_speed;	// [m/s]

	float m_noiseConeAngle;	// Defines a cone angle of noise added to direction


public:
	FlameParticle(oxygine::Resources& gameResources);
	void startEmitter(void);
	void stopEmitter(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
