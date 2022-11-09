#include "system.h"
#include "compoundobject.h"
#include "sceneactor.h"

#include "objectfactory.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

System::System(
   SceneActor* sceneActor,
   b2World* world,
   CompoundObject* parent) :
   m_sceneActor(sceneActor),
   m_world(world),
   m_parent(parent)
{
}

System* System::initialiseSystemNode(
   SceneActor* sceneActor,
   b2World* world,
   CompoundObject* parent, 
   string systemType, 
   string name, 
   const xml_node& node,
   int groupIndex)
{
   if (systemType == "objectFactory")
   {
      // initiate a object factory
      ObjectFactory* of = new ObjectFactory(
         sceneActor,
         parent,
         world,
         node.child("objectFactoryProperties"));

      of->setName(name);

      return of;
   }
   else if (systemType == "flameEmitter")
   {
      FlameEmitter* fe = new FlameEmitter(
         sceneActor,
         parent,
         world,
         node.child("flameEmitterProperties"),
         groupIndex);

      fe->setName(name);

      return fe;

   }
   else if (systemType == "gun")
   {
      Gun* g = new Gun(
         sceneActor,
         parent,
         world,
         node.child("gunProperties"),
         groupIndex);

      g->setName(name);

      return g;

   }
   else if (systemType == "shield")
   {
      Shield* sh = new Shield(
         sceneActor,
         parent,
         world,
         node.child("shieldProperties"),
         groupIndex);

      sh->setName(name);

      return sh;
   }
   else if (systemType == "reentryFlameEmitter")
   {
      ReentryFlameEmitter* rfe = new ReentryFlameEmitter(
         sceneActor,
         parent,
         world,
         node.child("reentryFlameEmitterProperties"));

      rfe->setName(name);

      return rfe;
   }

   return NULL;
}

CompoundObject* System::getParent(void)
{
   return m_parent;
}


