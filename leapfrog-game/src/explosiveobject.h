#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "spawnobject.h"

class SceneActor;


DECLARE_SMART(ExplosiveObject, spExplosiveObject);


// The ExplosiveObject class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the ExplosiveObject 
// Actor.

class ExplosiveObject : public CompoundObject
{
private:

   /*
   Lets define properties:
   Prop     Description
   0     =  Impulse threshold
   */

   enum properties
   {
      implThreshold = 0
   };

   enum ExplosionState
   {
      idle,
      armExplosion,
      waitForExplosionEnd
   };

protected:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   SceneActor* m_sceneActor;
   int m_blastDuration;

   ExplosionState m_state;
   oxygine::timeMS m_blastStartTime;

   // Below time is needed to calculate the damage of a particle
   // which could be called when current time is not known.
   oxygine::timeMS m_blastProgress;
   std::vector<b2Body*> m_blastParticleBodies;
   oxygine::spSprite m_explosionAnimation;

   int m_numOfParticles;
   float m_particlesRadius;
   float m_blastPower;
   float m_damageBulletEqv;
   bool m_impactExplosion;

   void readExplosiveObjectNode(const pugi::xml_node& node);
   void doExplosion(void);
   void startAnimation(b2Vec2 pos);

public:
	ExplosiveObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      const pugi::xml_node& root,
      const pugi::xml_node& propNode,
      int groupIndex);


   void triggerExplosion(void);
   float getDamageBulletEqv(void);
   void hitImpulse(const b2ContactImpulse* impulse, bool leapfrog = false);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};
