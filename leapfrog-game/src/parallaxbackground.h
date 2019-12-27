#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"

class SceneActor;


DECLARE_SMART(ParallaxBackground, spParallaxBackground);


// The ExplosiveObject class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the ExplosiveObject 
// Actor.

class ParallaxBackground : public CompoundObject
{
public:
   oxygine::spSprite m_sprite;
   float m_parallaxAmount;

protected:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   SceneActor* m_sceneActor;
 
   void readParallaxBackgroundObjectNode(const pugi::xml_node& node);

public:
   ParallaxBackground(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& root,
      int groupIndex);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};