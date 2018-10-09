#include "compoundobject.h"
#include "freespaceactor.h"
#include "objectfactory.h"
#include "actoruserdata.h"

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
   m_timeSinceLast(0),
   m_spawnCount(0)
{
   readObjectFactoryNode(objectNode);
 //  m_spawnObjects.readSpawnObjectsNode(objectNode);
   attachTo(sceneParent);
   spawnObjects();
}

ObjectFactory::~ObjectFactory()
{
   delete m_coNodeHolder;
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

   m_spawnCount = m_initialSpawn;
}

CollisionEntityTypeEnum ObjectFactory::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void ObjectFactory::doUpdate(const oxygine::UpdateState& us)
{
   m_timeSinceLast += us.dt;

   if (m_timeSinceLast >= m_interval)
   {
      m_spawnCount++;

      m_timeSinceLast = 0;
   }

   if (m_sceneActor)
   {
      spawnObjects();
   }
}

void ObjectFactory::spawnObjects(void)
{
   //for (int i = 0; i < m_spawnCount; i++)
   //{
   //   xml_node* spawnNode = m_spawnObjects.getSpawnObjectNode();

   //   if (spawnNode == NULL)
   //   {
   //      return;
   //   }

   //   CompoundObject* co = m_parent->defineChildObject(
   //      *m_gameResources,
   //      m_sceneActor,
   //      m_parent,
   //      m_world,
   //      PhysDispConvert::convert(m_parent->getCompoundObjectPosition(), 1.0f),
   //      *spawnNode,
   //      "");

   //   if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
   //   {
   //      FreeSpaceActor* spaceActor = static_cast<FreeSpaceActor*>(m_sceneActor);

   //      spaceActor->addBoundingBody(ActorUserData::getBody(co->getUserData()));
   //   }

   //}

   //m_spawnCount = 0;


}
