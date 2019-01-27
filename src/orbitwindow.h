#pragma once

#include "oxygine-framework.h"
#include "Box2d/Box2D.h"
#include "sceneactor.h"


DECLARE_SMART(OrbitWindow, spOrbitWindow);

class OrbitWindow : public CompoundObject
{
public:
   OrbitWindow();
   
   OrbitWindow(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      const pugi::xml_node& objectNode,
	  const std::string& initialState);

   // Initiate used when not defined from XML
   void InitiateOrbitWindow(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      float posX,
      float posY,
      float width,
      float height,
      float frameThickness,
      float spaceZoomScale,
      const std::string& spaceBackground,
      const std::string& spaceSceneFile,
      const std::string& state);

protected:
   void doUpdate(const oxygine::UpdateState &us);
   void generateBackground(Resources& gameResources);

};
