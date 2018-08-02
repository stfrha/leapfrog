#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(ObjectFactory, spObjectFactory);

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

class ObjectFactory : public CompoundObject
{
private:
   oxygine::Resources* m_gameResources;
   oxygine::Actor* m_parent;
   b2World* m_world;
   SceneActor* m_sceneActor;
   b2Vec2 m_leftTop;
   b2Vec2 m_rightBottom;

   int m_initialSpawn;

   // Time between spawned asteroids
   float m_interval;

   pugi::xml_document* m_coNodeHolder;
   pugi::xml_node m_coNode;

   // The time, from the creation until the field stop spawning new asteroids
	int m_fieldLifetime; // [ms}

   int m_timeSinceLast;

   std::vector<ObjectSpawnInstruction>  m_objectSpawnList;
   
   void readObjectFactoryNode(pugi::xml_node& objectNode);
   void spawnObjects(void);
   
public:
   ObjectFactory(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      pugi::xml_node& objectNode);

   void addObjectSpawnInstruction(ObjectSpawnInstruction& inst);

   virtual CollisionEntityTypeEnum getEntityType(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
