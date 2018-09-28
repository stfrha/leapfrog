#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

// A SpawnObject is not an instance of a object to spawn but a definition of
// what object to spawn. There can be multiple SpawnObject in a SpawnObjectList
// and when commanded to spawn, one of the SpawnObjects are choosed acc. to the 
// probability factor

class SpawnObject
{
public:
   int m_probabilityFactor;
   pugi::xml_node m_node;
   pugi::xml_document m_nodeDocHolder;

   SpawnObject(int probabilityFactor, const pugi::xml_node& node) :
      m_probabilityFactor(probabilityFactor)
   {
      pugi::xml_node tempNode = node.child("spawnObject");
      m_nodeDocHolder.append_copy(tempNode);
      m_node = m_nodeDocHolder.child("spawnObject");
   }
};


class SpawnObjectList
{
public:
   int m_probabilitySum;
   std::vector<SpawnObject*> m_spawnObjects;

   SpawnObjectList() :
      m_probabilitySum(0)
   {   }

   ~SpawnObjectList()
   {
      for (int i = 0; i < m_spawnObjects.size(); i++)
      {
         delete m_spawnObjects[i];
      }
   }

   void readSpawnObjectsNode(const pugi::xml_node& node)
   {
      for (auto it = node.children("spawnObject").begin(); it != node.children("spawnObject").end(); ++it)
      {
         int probFactor = it->attribute("probabilityFactor").as_int();
         SpawnObject* so = new SpawnObject(probFactor, (*it));
         m_probabilitySum += probFactor;
         m_spawnObjects.push_back(so);
      }
   }

   pugi::xml_node* getSpawnObjectNode(void)
   {
      if (m_spawnObjects.size() == 0)
      {
         return NULL;
      }

      // Randomise value between 0 and m_probabilitySum
      int r = rand() % m_probabilitySum;

      for (int i = 0; i < m_spawnObjects.size(); i++)
      {
         if (m_spawnObjects[i]->m_probabilityFactor <= r)
         {
            return &m_spawnObjects[i]->m_node;
         }
      }

      return &m_spawnObjects[m_spawnObjects.size() - 1]->m_node;
   }
};

