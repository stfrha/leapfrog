#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"


DECLARE_SMART(OrbitWindow, spOrbitWindow);

class OrbitWindow : public CompoundObject
{
public:
	OrbitWindow(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      pugi::xml_node& objectNode,
      std::string& initialState);

   virtual CollisionEntityTypeEnum getEntityType(void);

protected:
   void doUpdate(const oxygine::UpdateState &us);
   void generateBackground(Resources& gameResources);

};
