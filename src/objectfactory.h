#pragma once

#include "system.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"

class ObjectSpawnInstruction
{
public:
   int m_num;
   b2Vec2 m_leftTop;
   b2Vec2 m_rightBottom;

   ObjectSpawnInstruction() :
      m_num(0),
      m_leftTop(b2Vec2(0.0f, 0.0f)),
      m_rightBottom(b2Vec2(0.0f, 0.0f))
   {
   }

   ObjectSpawnInstruction(int num, b2Vec2 leftTop, b2Vec2 rightBottom) :
      m_num(num),
      m_leftTop(leftTop),
      m_rightBottom(rightBottom)
   {
   }
};

DECLARE_SMART(ObjectFactory, spObjectFactory);

class CompoundObject;
class SceneActor;

class ObjectFactory : public System
{
private:
   // Parameters from XML
   b2Body * m_attachedBody;

   b2Vec2 m_leftTop;
   b2Vec2 m_rightBottom;

   // Time between spawned asteroids
   float m_interval;

   // The time, from the creation until the field stop spawning new asteroids
   int m_fieldLifetime; // [ms}

   int m_initialSpawn;

   // Working member variable
   pugi::xml_document* m_coNodeHolder;
   pugi::xml_node m_coNode;
   int m_timeSinceLast;
   SpawnObjectList m_spawnObjects;
   int m_spawnCount;


   void readObjectFactoryNode(const pugi::xml_node& objectNode);
   void spawnObjects(void);
   
public:
   ObjectFactory(
      oxygine::Resources* gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode);

   ~ObjectFactory();

   virtual CollisionEntityTypeEnum getEntityType(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
