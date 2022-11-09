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
   b2World* m_world;
   SceneActor* m_sceneActor;
 
public:
   ParallaxBackground(
      SceneActor* sceneParent,
      b2World* world,
      float parallaxAmount);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};
