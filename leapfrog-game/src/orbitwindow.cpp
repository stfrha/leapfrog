#include <algorithm>

#include "orbitspacescene.h"

#include "orbitwindow.h"


using namespace oxygine;
using namespace pugi;
using namespace std;

OrbitWindow::OrbitWindow(
   Resources& gameResources,
   CompoundObject* parentObject,
   b2World* world,
   float posX,
   float posY,
   float width,
   float height,
   float frameThickness,
   float spaceZoomScale,
   const string& spaceBackground,
   const string& spaceSceneFile,
   const string& state) :
   CompoundObject((SceneActor*)this, parentObject)
{
   // Set up clip window which becoms the boundary for the SceneActor
   // Build closeup of Leapfrog

   spColorRectSprite frame = new ColorRectSprite();
   frame->setSize(width, height);

   frame->setPosition(posX, posY);

   frame->attachTo(this);

   spClipRectActor clip = new ClipRectActor();
   clip->setSize(width - frameThickness * 2.0f, height - frameThickness * 2.0f);
   clip->setPosition(frameThickness, frameThickness);
   clip->attachTo(frame);

   spSceneActor sceneObj = static_cast<SceneActor*>(CompoundObject::readDefinitionXmlFile(
      gameResources,
      NULL,
      NULL,
      world,
      Vector2(0.0f, 0.0f),
      spaceSceneFile,
      string("default")));

   sceneObj->setZoom(spaceZoomScale);

   //spOrbitSpaceScene orbitSpaceActor = new OrbitSpaceScene(
   //   gameResources,
   //   spaceZoomScale,
   //   spaceBackground,
   //   spaceSceneFile,
   //   string("deepSpaceState"));

   clip->addChild(sceneObj);

   m_children.push_back(static_cast<CompoundObject*>(sceneObj.get()));

   sceneObj->setName("spaceScene");

   attachTo(parentObject);
}

void OrbitWindow::doUpdate(const oxygine::UpdateState &us)
{
}


void OrbitWindow::generateBackground(Resources& gameResources)
{
   //for (int x = 0; x < 5; x++)
   //{
   //   for (int y = 0; y < 3; y++)
   //   {
   //      spSprite background = new Sprite();
   //      background->setResAnim(gameResources.getResAnim("starfield"));
   //      background->setPosition(-150.0f + 256.0f * x, -150.0f + 256.0f * y);
   //      background->setSize(512.0f, 512.0f);
   //      background->setScale(0.5f);
   //      background->setPriority(26);
   //      background->attachTo(this);
   //   }
   //}

}

