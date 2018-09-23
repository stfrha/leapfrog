#include "system.h"
#include "compoundobject.h"
#include "sceneactor.h"

#include "objectfactory.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

System::System(
   oxygine::Resources* gameResources,
   SceneActor* sceneActor,
   b2World* world,
   CompoundObject* parent) :
   m_gameResources(gameResources),
   m_sceneActor(sceneActor),
   m_world(world),
   m_parent(parent)
{
}

System* System::initialiseSystemNode(
   Resources* gameResources,
   SceneActor* sceneActor,
   b2World* world,
   CompoundObject* parent, 
   string systemType, 
   string name, 
   const xml_node& node)
{
   if (systemType == "objectFactory")
   {
      // initiate a object factory
      ObjectFactory* of = new ObjectFactory(
         gameResources,
         sceneActor,
         parent,
         world,
         node.child("properties"));

      of->setName(name);

      return of;
   }
   else if (systemType == "flameEmitter")
   {
      FlameEmitter* fe = new FlameEmitter(
         gameresources,
         sceneActor,
         parent,
         world,
         node.child("properties"));

      fe->setName(name);

   }
   else if (systemType == "gun")
   {

   }
   else if (systemType == "shield")
   {

   }

   return NULL;
}



