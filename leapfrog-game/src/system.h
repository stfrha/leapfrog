#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"

class SceneActor;
class CompoundObject;

DECLARE_SMART(System, spSystem);

class System : public oxygine::Actor
{
public:
   enum SystemTypeEnum
   {
      objectFactory,
      flameEmitter,
      gun,
      shield
   };

protected:
   SceneActor* m_sceneActor;
   b2World* m_world;
   CompoundObject* m_parent;

public:
   System(
      SceneActor* sceneActor,
      b2World* world,
      CompoundObject* parent);

   static System* initialiseSystemNode(
      SceneActor* sceneActor,
      b2World* world,
      CompoundObject* parent,
      std::string systemType,
      std::string name,
      const pugi::xml_node& node,
      int groupIndex);

   CompoundObject* getParent(void);

};
