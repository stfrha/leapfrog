
#include "spawnobject.h"

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

   for (unsigned int i = 0; i < m_spawnObjects.size(); i++)
   {
      if (m_spawnObjects[i]->m_probabilityFactor <= r)
      {
         return &m_spawnObjects[i]->m_node;
      }
   }

   return &m_spawnObjects[m_spawnObjects.size() - 1]->m_node;

   return NULL;
}


SpawnInstruction::SpawnInstruction(
   int numOfSpawns,
   oxygine::Vector2 leftTop,
   oxygine::Vector2 widthHeight,
   spSpawnObjectList spawnSource) :
   m_numOfSpawns(numOfSpawns),
   m_leftTop(leftTop),
   m_widthHeight(widthHeight),
   m_spawnSource(spawnSource)
{
}
