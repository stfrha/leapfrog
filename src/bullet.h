#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "actortodie.h"

class SceneActor;

DECLARE_SMART(Bullet, spBullet);

class Bullet : public CompoundObject
{
private:
   int m_deathTime;
   SceneActor* m_sceneActor;

   int m_debugTimeAtCreation;
   int m_debugHistoryOfTime[10];

   bool m_firstUpdate;
   int m_lifetime;

public:
   Bullet(
      oxygine::Resources& gameResources,
      SceneActor* sceneActor,
      b2World* world,
      const b2Vec2& pos,
      const float angle,
      float impulseMagnitude,
      b2Vec2& craftSpeed,
      int m_lifetime,
      bool bouncy);

   CollisionEntityTypeEnum Bullet::getEntityType(void);

   virtual void killActor(void);

   void hitAsteroid(b2Contact* contact);


protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atBulletDeath(void);
};
