#pragma once

#include "system.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"

DECLARE_SMART(ObjectFactory, spObjectFactory);

class CompoundObject;
class SceneActor;

class ObjectFactory : public System
{
private:
   // Parameters from XML
   b2Body * m_attachedBody;

   oxygine::Vector2 m_leftTop;
   oxygine::Vector2 m_widthHeight;

   // Time between spawned asteroids
   float m_interval;

   // The time, from the creation until the field stop spawning new asteroids
   int m_fieldLifetime; // [ms}

   int m_initialSpawn;

   // Working member variable
   pugi::xml_document* m_coNodeHolder;
   pugi::xml_node m_coNode;
   int m_timeSinceLast;
   spSpawnObjectList m_spawnObjects;

   void readObjectFactoryNode(const pugi::xml_node& objectNode);
   void spawnObjects(int num);
   
public:
   ObjectFactory(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode);

   ~ObjectFactory();

protected:
	void doUpdate(const oxygine::UpdateState& us) override;

};
