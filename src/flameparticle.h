#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(FlameParticle, spFlameParticle);

class FlameParticle : public oxygine::Sprite
{
private:

public:
	FlameParticle(oxygine::Resources& gameResources, b2World* world, const b2Vec2& pos, int m_lifetime, b2Vec2 m_impulseForce, float radius);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atParticleDeath(oxygine::Event* event);
};
