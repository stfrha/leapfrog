#include <algorithm>

#include "orbitspacescene.h"

#include "orbitwindow.h"


using namespace oxygine;
using namespace pugi;
using namespace std;

OrbitWindow::OrbitWindow(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   xml_node& objectNode,
   string& initialState) 
{
   // Set up clip window which becoms the boundary for the SceneActor
   // Build closeup of Leapfrog
   float width = objectNode.attribute("width").as_float();
   float height = objectNode.attribute("height").as_float();
   float frameThickness = objectNode.attribute("frameThickness").as_float();

   spColorRectSprite frame = new ColorRectSprite();
   frame->setSize(width, height);

   frame->setPosition(
      objectNode.attribute("posX").as_float(),
      objectNode.attribute("posY").as_float());

   frame->attachTo(this);

   spClipRectActor clip = new ClipRectActor();
   clip->setSize(width - frameThickness * 2.0f, height - frameThickness * 2.0f);
   clip->setPosition(frameThickness, frameThickness);
   clip->attachTo(frame);

   //spSprite background = new Sprite();
   //background->setResAnim(gameResources.getResAnim(objectNode.attribute("background").as_string()));
   //background->setSize(800.0f, 300.0f);
   //background->setPosition(-150.0f, 0.0f);
   //background->setScale(0.3f);
   //background->setPriority(26);
   //background->attachTo(clip);

   //m_lfStage = new Actor();
   //m_lfStage->setSize(300.0f, 150.0f);
   //m_lfStage->setAnchor(0.5f, 0.5f);
   //m_lfStage->setPosition(-50.0f, -20.0f);
   //m_lfStage->attachTo(clip);

   spOrbitSpaceScene orbitSpaceActor = new OrbitSpaceScene(
      gameResources, 
      objectNode.attribute("zoomScale").as_float(),
      (string)objectNode.attribute("background").as_string(),
      (string)objectNode.attribute("spaceSceneFile").as_string(),
      string("deepSpaceState"));
   clip->addChild(orbitSpaceActor);

   m_children.push_back(static_cast<CompoundObject*>(orbitSpaceActor.get()));

   orbitSpaceActor->setName("spaceScene");

   attachTo(sceneParent);
   // Create background before the leapfrog
   // generateBackground(gameResources);

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

