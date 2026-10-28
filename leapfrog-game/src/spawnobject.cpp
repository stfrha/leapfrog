
#include "spawnobject.h"
#include "compoundobject.h"

SpawnObject::SpawnObject(int probabilityFactor, const pugi::xml_node& node) :
   m_probabilityFactor(probabilityFactor)
{
   m_nodeDocHolder.append_copy(node);
   m_node = m_nodeDocHolder.child("spawnObject").child("childObject");
}

SpawnObjectList::SpawnObjectList() :
   m_probabilitySum(0)
{   }

void SpawnObjectList::readSpawnObjectsNode(const pugi::xml_node& node)
{
   for (auto it = node.children("spawnObject").begin(); it != node.children("spawnObject").end(); ++it)
   {
      int probFactor = it->attribute("probabilityFactor").as_int();
      pugi::xml_node n = (*it);
      spSpawnObject so = new SpawnObject(probFactor, n);
      m_probabilitySum += probFactor;
      m_spawnObjects.push_back(so);
   }
}

pugi::xml_node* SpawnObjectList::getSpawnObjectNode(void)
{
   if (m_spawnObjects.size() == 0)
   {
      return NULL;
   }

   // Randomise value between 0 and m_probabilitySum
   int r = rand() % m_probabilitySum;
   int partSum = 0;

   for (unsigned int i = 0; i < m_spawnObjects.size(); i++)
   {
      int prob = m_spawnObjects[i]->m_probabilityFactor;

      if ((r >= partSum) && (r < prob + partSum))
      {
         return &m_spawnObjects[i]->m_node;
      }
      partSum += prob;
   }

   return &m_spawnObjects[m_spawnObjects.size() - 1]->m_node;
}


SpawnInstruction::SpawnInstruction(
   int numOfSpawns,
   oxygine::Vector2 leftTop,
   oxygine::Vector2 widthHeight,
   spSpawnObjectList spawnSource,
   spSpawnTally tally) :
   m_numOfSpawns(numOfSpawns),
   m_leftTop(leftTop),
   m_widthHeight(widthHeight),
   m_spawnSource(spawnSource),
   m_tally(tally)
{
   if (m_tally)
   {
      m_tally->m_pending += numOfSpawns;
   }
}


SpawnTally::SpawnTally() :
   m_pending(0)
{
}

void SpawnTally::add(CompoundObject* object)
{
   if (object == NULL)
   {
      return;
   }

   Entry e;
   e.m_actor = object;
   e.m_object = object;
   e.m_attached = (object->getParent() != NULL);
   m_objects.push_back(e);
}

// An object is gone when it is dead or has been taken off the scene
// (death list). Objects without a behaviour are never attached to the
// scene and never die, so they count as alive for good.
int SpawnTally::countAlive(void)
{
   for (auto it = m_objects.begin(); it != m_objects.end(); )
   {
      bool alive = !it->m_object->m_isDead &&
         (!it->m_attached || (it->m_object->getParent() != NULL));

      if (alive)
      {
         ++it;
      }
      else
      {
         it = m_objects.erase(it);
      }
   }

   return (int)m_objects.size() + m_pending;
}
