#include <algorithm>

#include "compoundobject.h"
#include "freespaceactor.h"
#include "objectfactory.h"
#include "actoruserdata.h"
#include "mainactor.h"

using namespace oxygine;
using namespace pugi;
using namespace std;

ObjectFactory::ObjectFactory(
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& objectNode) :
   System(sceneParent, world, parentObject),
   m_timeSinceLast(0.0f),
   m_initialSpawn(0),
   m_maxAlive(0),
   m_tally(NULL),
   m_spawnTotal(0),
   m_spawnedSoFar(0),
   m_leapfrogClearance(c_defaultLeapfrogClearance),
   m_watchLeft(-1),
   m_watchId(0),
   m_active(true),
   m_initialPending(false),
   m_intensity(0.0f),
   m_attachedBody(NULL)
{
   readObjectFactoryNode(objectNode);

   m_spawnObjects = new SpawnObjectList();

   m_spawnObjects->readSpawnObjectsNode(objectNode);

   attachTo(sceneParent);

   if (m_active)
   {
      spawnObjects(m_initialSpawn);
   }
   else
   {
      m_initialPending = true;
   }
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
   setIntensity(objectNode.attribute("intensity").as_float());
   m_active = objectNode.attribute("active").as_bool(true);
   m_fieldLifetime = objectNode.attribute("lifeTime").as_int();
   m_maxAlive = objectNode.attribute("maxAlive").as_int(0);
   m_spawnTotal = objectNode.attribute("spawnTotal").as_int(0);
   m_leapfrogClearance = objectNode.attribute("leapfrogClearance").as_float(c_defaultLeapfrogClearance);

   m_tally = new SpawnTally();

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

void ObjectFactory::setIntensity(float perSecond)
{
   m_intensity = (perSecond > 0.0f) ? perSecond : 0.0f;

   // No timed spawns at intensity 0
   m_interval = (m_intensity > 0.0f) ? 1000.0f / m_intensity : 0.0f;
}

void ObjectFactory::doUpdate(const oxygine::UpdateState& us)
{
   if (!g_MainActor->isInPause())
   {
      if (m_active && (m_interval > 0.0f))
      {
         m_timeSinceLast += us.dtf * 1000.0f;

         if (m_timeSinceLast >= m_interval)
         {
            spawnObjects(1);

            m_timeSinceLast = 0.0f;
         }
      }

      // A factory that hasn't started yet isn't nearly empty
      bool allSpawned = !m_initialPending &&
         ((m_spawnTotal == 0) || (m_spawnedSoFar >= m_spawnTotal));

      if ((m_watchLeft >= 0) && allSpawned && (m_tally->countAlive() <= m_watchLeft))
      {
         m_watchLeft = -1;

         FactoryLowEvent event(m_watchId);
         m_sceneActor->dispatchEvent(&event);
      }
   }
}

void ObjectFactory::watch(int left, int id)
{
   m_watchLeft = left;
   m_watchId = id;
}

bool ObjectFactory::setFactoryProperty(int id, float value)
{
   switch (id)
   {
   case propActive:
   {
      bool active = (value != 0.0f);

      if (active && !m_active)
      {
         // The first timed spawn comes one interval after activation
         m_timeSinceLast = 0.0f;
      }

      m_active = active;

      if (m_active && m_initialPending)
      {
         m_initialPending = false;
         spawnObjects(m_initialSpawn);
      }
      return true;
   }
   case propIntensity:
      setIntensity(value);
      return true;
   case propMaxAlive:
      m_maxAlive = std::max(0, (int)value);
      return true;
   case propSpawnTotal:
      m_spawnTotal = std::max(0, (int)value);
      return true;
   case propSpawned:
      m_spawnedSoFar = std::max(0, (int)value);
      return true;
   case propSpawnNow:
      spawnObjects((int)value);
      return true;
   default:
      // propAlive is read only
      return false;
   }
}

bool ObjectFactory::getFactoryProperty(int id, float& value)
{
   switch (id)
   {
   case propActive:     value = m_active ? 1.0f : 0.0f; return true;
   case propIntensity:  value = m_intensity; return true;
   case propMaxAlive:   value = (float)m_maxAlive; return true;
   case propSpawnTotal: value = (float)m_spawnTotal; return true;
   case propAlive:      value = (float)m_tally->countAlive(); return true;
   case propSpawned:    value = (float)m_spawnedSoFar; return true;
   case propSpawnNow:   value = 0.0f; return true;
   default:             return false;
   }
}

void ObjectFactory::spawnObjects(int num)
{
   if (m_maxAlive > 0)
   {
      num = std::min(num, m_maxAlive - m_tally->countAlive());
   }

   if (m_spawnTotal > 0)
   {
      num = std::min(num, m_spawnTotal - m_spawnedSoFar);
   }

   if (num <= 0)
   {
      return;
   }

   m_spawnedSoFar += num;

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
      m_spawnObjects,
      m_tally,
      Vector2(0.0f, 0.0f),
      m_leapfrogClearance);

}
