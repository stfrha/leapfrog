#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "system.h"
#include "collisionentity.h"

#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(Shield, spShield);

class Shield : public System, CollisionEntity
{
private:
   // Parameters from XML
   b2Body * m_attachedBody;

   // Seems that only the center of the attachedBody is used.
   // Therefore we don't use origin now, lets see if this 
   // can be corrected
   // b2Vec2 m_attachedOrigin;
   
   float m_radius;

   // Working member variable
   float m_angle;

   oxygine::spSprite m_sprite;

   // Held up by a docking station: the bubble stays on whatever the
   // Leapfrog's shield resource is, and hits only flash it (no drain)
   bool m_heldUp;

   void readShieldNode(const pugi::xml_node& objectNode);
   void shieldHit(b2Contact* contact);
   void evaluatShieldDamage(void);

public:
   // Members to access from other components
   b2Body * m_body;
   b2RevoluteJoint* m_shieldJoint;

   oxygine::ResAnim* m_resAnim;
   
   Shield(
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode,
      int groupIndex);
   
   void setAngle(float angle);
   float getAngle(void);

   void shieldHitImpulse(b2Contact* contact, const b2ContactImpulse* impulse);
   void shieldHitByBullet(b2Contact* contact, float bulletEqvDamage);
   void restartShield(void);
   void stopShield(void);

   // A docking station holds the bubble up for its docked client
   void setHeldUp(bool heldUp);
   bool isHeldUp(void) const { return m_heldUp; }

protected:
	void doUpdate(const oxygine::UpdateState& us) override;
   void atHitComplete(oxygine::Event* event);
};
