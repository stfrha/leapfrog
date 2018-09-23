#include "freespaceactor.h"
#include "asteroid.h"
#include "objectfactory.h"

using namespace oxygine;
using namespace pugi;
using namespace std;

ObjectFactory::ObjectFactory(
   Resources* gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& objectNode) :
   System(gameResources, sceneParent, world, parentObject),
   m_timeSinceLast(0)
{
   readObjectFactoryNode(objectNode);
   attachTo(sceneParent);
   spawnObjects();
}

void ObjectFactory::readObjectFactoryNode(const xml_node& objectNode)
{
   float posX = objectNode.attribute("posX").as_float();
   float posY = objectNode.attribute("posY").as_float();
   float width = objectNode.attribute("width").as_float();
   float height = objectNode.attribute("height").as_float();
   m_leftTop = b2Vec2(posX - width / 2, posY - height / 2);
   m_rightBottom = b2Vec2(posX + width / 2, posY + height / 2);
   m_initialSpawn = objectNode.attribute("spawnInitial").as_int();
   m_interval = 1.0f / objectNode.attribute("intensity").as_float() * 1000.0f;
   m_fieldLifetime = objectNode.attribute("lifeTime").as_int();

   m_attachedBody = m_parent->getBody(objectNode.attribute("body").as_string());

   m_coNode = objectNode.child("spawnObject");
   m_coNodeHolder = new xml_document();
   m_coNodeHolder->append_copy(m_coNode);
   m_coNode = m_coNodeHolder->child("spawnObject");

   addObjectSpawnInstruction(ObjectSpawnInstruction(m_initialSpawn, m_leftTop, m_rightBottom ));

}

CollisionEntityTypeEnum ObjectFactory::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}



void ObjectFactory::addObjectSpawnInstruction(const ObjectSpawnInstruction& inst)
{
   m_objectSpawnList.push_back(inst);
}


void ObjectFactory::doUpdate(const oxygine::UpdateState& us)
{
   m_timeSinceLast += us.dt;

   if (m_timeSinceLast >= m_interval)
   {
      addObjectSpawnInstruction(ObjectSpawnInstruction(1, m_leftTop, m_rightBottom));

      m_timeSinceLast = 0;
   }

   if (m_sceneActor)
   {
      spawnObjects();
   }
}

void ObjectFactory::spawnObjects(void)
{
   for (auto it = m_objectSpawnList.begin(); it != m_objectSpawnList.end(); ++it)
   {
      for (int i = 0; i < it->m_num; i++)
      {
         // If m_attachedBody is NULL, the position is absolute in scene-space

         // Randomise position within factory field
         Vector2 pos;

         pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.x  - it->m_leftTop.x))) + it->m_leftTop.x;
         pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.y - it->m_leftTop.y))) + it->m_leftTop.y;

         // If there is a attached body, the position is relative to the body position.
         if (m_attachedBody)
         {
            b2Vec2 originPos = m_attachedBody->GetWorldPoint(PhysDispConvert::convert(pos, 1.0f));
            pos = PhysDispConvert::convert(originPos, 1.0f);
         }

         spCompoundObject co = new CompoundObject(m_sceneActor);

         co = co->initCompoundObject(*m_gameResources, m_sceneActor, m_sceneActor, m_world, pos, m_coNode, string("default"));
         
         co->attachTo(m_sceneActor);

         if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
         {
            FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

            spaceActor->addBoundingBody((b2Body*)co.get()->getUserData());
         }
      }
   }

   m_objectSpawnList.clear();
}

