#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

DECLARE_SMART(FlameParticle, spFlameParticle);

class FlameParticle : public oxygine::Sprite, CollisionEntity
{
private:
   oxygine::Resources * m_gameResources;
   b2World* m_world;
   b2Body* m_body;

public:
	FlameParticle(
      oxygine::Resources& gameResources, 
      b2World* world, 
      const b2Vec2& pos, 
      const b2Vec2& vel, 
      int m_lifetime, 
      b2Vec2 m_impulseForce, 
      float radius,
      int groupIndex);


protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atParticleDeath(oxygine::Event* event);
};
