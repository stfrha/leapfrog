#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"


DECLARE_SMART(OrbitWindow, spOrbitWindow);

class OrbitWindow : public CompoundObject
{
public:
   OrbitWindow();
   
   OrbitWindow(
      CompoundObject* parentObject,
      b2World* world,
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
   void doUpdate(const oxygine::UpdateState &us) override;
   void generateBackground(Resources& gameResources);

};
