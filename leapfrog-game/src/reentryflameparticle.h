#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(ReentryFlameParticle, spReentryFlameParticle);

class ReentryFlameParticle : public oxygine::Sprite
{
private:

public:
	ReentryFlameParticle(
      oxygine::Resources* gameResources,
      const oxygine::Vector2& pos, 
      int m_lifetime, 
      const oxygine::Vector2& growToSize);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atParticleDeath(oxygine::Event* event);
};
