#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "actortodie.h"

class SceneActor;

DECLARE_SMART(Bullet, spBullet);

class Bullet : public oxygine::ColorRectSprite, CollisionEntity, ActorToDie
{
private:
   float m_bulletSpeed;
   int m_deathTime;
   SceneActor* m_sceneActor;

public:
   Bullet(
      oxygine::Resources& gameResources,
      SceneActor* sceneActor,
      b2World* world,
      const b2Vec2& pos,
      const float angle,
      float bulletSpeed,
      int m_lifetime,
      bool bouncy);

   CollisionEntityTypeEnum Bullet::getEntityType(void);

   virtual void killActor(void);

   void hitAsteroid(b2Contact* contact);


protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atBulletDeath(void);
};
