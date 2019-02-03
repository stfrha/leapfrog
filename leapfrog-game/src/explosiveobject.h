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

   enum ExplosionState
   {
      idle,
      armExplosion,
      waitForExplosionEnd
   };


   oxygine::Resources* m_gameResource;
   b2World* m_world;
   SceneActor* m_sceneActor;

   ExplosionState m_state;
   oxygine::timeMS m_blastTime;
   std::vector<b2Body*> m_blastParticleBodies;
   oxygine::spSprite m_explosionAnimation;

   int m_numOfParticles;
   float m_particlesRadius;
   float m_blastPower;

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
      pugi::xml_node& root,
      int groupIndex);


   void triggerExplosion(void);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};
