#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

DECLARE_SMART(FlameSmokeParticle, spFlameSmokeParticle);

class FlameSmokeParticle : public oxygine::Sprite, CollisionEntity
{
private:
   oxygine::Resources * m_gameResources;

   oxygine::Vector2 m_originalDirection;
   b2Body* m_body;
   b2Vec2 m_antiGravForce;

public:
	FlameSmokeParticle(
      oxygine::Resources& gameResources, 
      b2World* world, 
      const b2Vec2& pos,      // Start position
      const b2Vec2& vel,      // Start direction and speed that will be decellerated by an opposing force
      int m_lifetime, 
      float radius);          // start Radius, will get scaled up and thinned (alpha) down

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atParticleDeath(oxygine::Event* event);
};
