#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

class SceneActor;

DECLARE_SMART(Bullet, spBullet);

class Bullet : public oxygine::ColorRectSprite, CollisionEntity
{
private:
   int m_deathTime;
   SceneActor* m_sceneActor;

   int m_debugTimeAtCreation;
   int m_debugHistoryOfTime[10];

   bool m_firstUpdate;
   int m_lifetime;

   int m_groupIndex;

public:
   Bullet(
      SceneActor* sceneActor,
      b2World* world,
      const b2Vec2& pos,
      const float angle,
      float impulseMagnitude,
      b2Vec2& craftSpeed,
      int m_lifetime,
      bool bouncy,
      int groupIndex);

   // I need at least one virtual member to make the 
   // the class work with dynamic_cast
   virtual ~Bullet()
   {}


   //virtual void killActor(void);

   void bulletHit(b2Contact* contact);



protected:
	void doUpdate(const oxygine::UpdateState& us);
};
