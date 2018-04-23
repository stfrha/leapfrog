#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

DECLARE_SMART(Shield, spShield);

class Shield : public oxygine::Sprite, CollisionEntity
{
private:
   float m_angle;

public:
   b2Body * m_body;
   oxygine::ResAnim* m_resAnim;
   
   Shield(oxygine::Resources& gameResources, b2World* world, const oxygine::Vector2& pos);

   virtual CollisionEntityTypeEnum getEntityType(void);
   void setAngle(float angle);
   float getAngle(void);
   void shieldHit(b2Contact* contact);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atHitComplete(oxygine::Event* event);
};
