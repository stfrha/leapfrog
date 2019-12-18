
#include "parallaxbackground.h"

#include "bodyuserdata.h"
#include "sceneactor.h"
#include "freespaceactor.h"
#include "actoruserdata.h"

#include "groupindexsource.h"
#include "layout.h"

using namespace oxygine;
using namespace pugi;

ParallaxBackground::ParallaxBackground(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_gameResource(&gameResources),
   m_sceneActor(sceneParent),
   m_world(world)
{
   
   //readParallaxBackgroundNode(root.child("behaviour").child("ParallaxBackgroundProperties"));
   readParallaxBackgroundObjectNode(root.child("behaviour").child("parallaxBackgroundProperties"));

   m_sprite = new Sprite();
   m_sprite->setSize(g_Layout.getStageBounds().getSize());
   m_sprite->setAnchor(0.0f, 0.0f);
   m_sprite->setTouchChildrenEnabled(false);

   // z-level range from 20 (at 100% amount, i.e. static background,
   // to 30 which is foreground
   m_sprite->setPriority(30.0f - m_parallaxAmount * 10.0f);

   initCompoundObjectParts(
      gameResources,
      m_sprite.get(),
      sceneParent,
      parentObject,
      world,
      Vector2(0.0f, 0.0f),
      root,
      "",
      groupIndex);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   sceneParent->addParallaxBackground(this);
}

void ParallaxBackground::readParallaxBackgroundObjectNode(const pugi::xml_node& node)
{
   m_parallaxAmount = node.attribute("parallaxAmount").as_float(0.0f);
}


void ParallaxBackground::doUpdate(const oxygine::UpdateState& us)
{
}



