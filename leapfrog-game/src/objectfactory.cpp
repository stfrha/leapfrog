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
   m_initialSpawn(0),
   m_attachedBody(NULL)
{
   readObjectFactoryNode(objectNode);

   m_spawnObjects = new SpawnObjectList();

   m_spawnObjects->readSpawnObjectsNode(objectNode);

   attachTo(sceneParent);

   spawnObjects(m_initialSpawn);
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
   m_leftTop = Vector2(posX, posY);
   m_widthHeight = Vector2(width, height);
   m_initialSpawn = objectNode.attribute("spawnInitial").as_int();
   m_interval = 1.0f / objectNode.attribute("intensity").as_float() * 1000.0f;
   m_fieldLifetime = objectNode.attribute("lifeTime").as_int();

   string body = objectNode.attribute("body").as_string("notApplicable");
   
   if ((body != "") && (body != "notApplicable"))
   {
      m_attachedBody = m_parent->getBody(body);
   }
   else
   {
      m_attachedBody = NULL;
   }

   m_coNode = objectNode.child("spawnObject");
   m_coNodeHolder = new xml_document();
   m_coNodeHolder->append_copy(m_coNode);
   m_coNode = m_coNodeHolder->child("spawnObject");
}

void ObjectFactory::doUpdate(const oxygine::UpdateState& us)
{
   if (!m_sceneActor->getIsInPause())
   {
      m_timeSinceLast += us.dt;

      if (m_timeSinceLast >= m_interval)
      {
         spawnObjects(1);

         m_timeSinceLast = 0;
      }
   }
}

void ObjectFactory::spawnObjects(int num)
{
   // Find spawn position
   // If this ObjectFactory is attached to body,
   // the position is the body position + the pos specifier
   // in the XML.
   // If there is no attached body, the pos specifier
   // is the world-coordinates position to use   
   Vector2 pos;
   
   if (m_attachedBody)
   {
      // Position is relative to the body angle, so it must be rotated accordingly.
      // Find angle of body, rotate the local pos with this angle, and use that position
      pos = PhysDispConvert::convert(m_attachedBody->GetWorldPoint(PhysDispConvert::convert(m_leftTop, 1.0f)), 1.0f);
   }
   else
   {
      pos = m_leftTop;
   }

   m_sceneActor->addObjectToSpawnList(
      num,
      pos,
      m_widthHeight,
      m_spawnObjects);

}
