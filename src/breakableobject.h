#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "spawnobject.h"

class SceneActor;


DECLARE_SMART(BreakableObject, spBreakableObject);


// The BreakableObject class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the BreakableObject 
// Actor.

class BreakableObject : public CompoundObject
{
private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   SceneActor* m_sceneActor;
   SpawnObjectList m_spawnObjects;

   int m_breakAtDamage;
   int m_numberOfSpawns;

   int m_spawnCount;
   int m_damage;


   void readBreakableObjectNode(const pugi::xml_node& node);
   void spawnBreakableObjects(void);
   void addShapesToDeathList(void);

public:
	BreakableObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void hitByBullet(b2Contact* contact); 
   void hitShield(b2Contact* contact);     // Returns true if BreakableObject was shattered
   void hitByLepfrog(b2Contact* contact);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atDeathOfBreakableObject(void);
};
