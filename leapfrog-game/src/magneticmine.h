#pragma once

#include "explosiveobject.h"
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "spawnobject.h"

class SceneActor;


DECLARE_SMART(MagneticMine, spMagneticMine);


// The MagneticMine class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the MagneticMine 
// Actor.

class MagneticMine : public ExplosiveObject
{
private:
   float m_magneticDistance;
   float m_magneticForce;

   b2Body* m_magenticMineBody;

   void readMagneticMineNode(const pugi::xml_node& node);

public:
	MagneticMine(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};
