#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(BlastParticle, spBlastParticle);

class BlastParticle : public oxygine::Sprite
{
private:

public:
	BlastParticle(
      oxygine::Resources* gameResources,
      const oxygine::Vector2& pos, 
      int m_lifetime, 
      float distance, 
      float particleSize);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atParticleDeath(oxygine::Event* event);
};
