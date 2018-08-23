#include <algorithm>

#include "orbitspacescene.h"

#include "orbitwindow.h"


using namespace oxygine;
using namespace pugi;
using namespace std;

OrbitWindow::OrbitWindow() :
   CompoundObject((SceneActor*)this)
{
}

OrbitWindow::OrbitWindow(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   const xml_node& objectNode,
   const string& initialState) :
   CompoundObject((SceneActor*)this)
{
   InitiateOrbitWindow(
      gameResources,
      sceneParent,
      parentObject,
      objectNode.attribute("posX").as_float(),
      objectNode.attribute("posY").as_float(),
      objectNode.attribute("width").as_float(),
      objectNode.attribute("height").as_float(),
      objectNode.attribute("frameThickness").as_float(),
      objectNode.attribute("zoomScale").as_float(),
      objectNode.attribute("background").as_string(),
      objectNode.attribute("spaceSceneFile").as_string(),
      initialState);
}

void OrbitWindow::InitiateOrbitWindow(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   float posX,
   float posY,
   float width,
   float height,
   float frameThickness,
   float spaceZoomScale,
   const string& spaceBackground,
   const string& spaceSceneFile,
   const string& state)
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

   spOrbitSpaceScene orbitSpaceActor = new OrbitSpaceScene(
      gameResources,
      spaceZoomScale,
      spaceBackground,
      spaceSceneFile,
      string("deepSpaceState"));

   clip->addChild(orbitSpaceActor);

   m_children.push_back(static_cast<CompoundObject*>(orbitSpaceActor.get()));

   orbitSpaceActor->setName("spaceScene");

   attachTo(sceneParent);
}

CollisionEntityTypeEnum OrbitWindow::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
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

