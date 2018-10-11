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
   spSpawnObjectList m_spawnObjects;

   int m_breakAtDamage;
   int m_numberOfSpawns;

   int m_damage;


   void readBreakableObjectNode(const pugi::xml_node& node);
   void spawnBreakableObjects(void);

public:
	BreakableObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);



   void collisionBlast(b2Contact* contact, bool small = true);
   void damageCollision(b2Contact* contact, float bulletEqvDamage);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atDeathOfBreakableObject(void);
};
