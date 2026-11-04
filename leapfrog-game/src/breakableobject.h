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
   b2World* m_world;
   SceneActor* m_sceneActor;
   spSpawnObjectList m_spawnObjects;

   int m_breakAtDamage;
   int m_numberOfSpawns;
   bool m_calm;                  // calm="true": ignores the scene's rock speed
   bool m_still;                 // still="true": no nudge at all
   float m_linearDamping;        // on the first body (default 0)
   float m_angularDamping;

   void readBreakableObjectNode(const pugi::xml_node& node);
   void spawnBreakableObjects(void);

public:
	BreakableObject(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);



   void collisionBlast(b2Contact* contact, bool small = true);
   void damageCollision(b2Contact* contact, float bulletEqvDamage);
   void initObjectResources(oxygine::Actor* statusEventOriginator, ObjectResources* resources = NULL) override;


protected:
	void doUpdate(const oxygine::UpdateState& us) override;
   void atDeathOfBreakableObject(void);
};
