#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

// A SpawnObject is not an instance of a object to spawn but a definition of
// what object to spawn. There can be multiple SpawnObject in a SpawnObjectList
// and when commanded to spawn, one of the SpawnObjects are choosed acc. to the 
// probability factor

DECLARE_SMART(SpawnObject, spSpawnObject);

class SpawnObject : public oxygine::ref_counter
{
public:
   int m_probabilityFactor;
   pugi::xml_node m_node;
   pugi::xml_document m_nodeDocHolder;
   
   SpawnObject(int probabilityFactor, const pugi::xml_node& node);
};



DECLARE_SMART(SpawnObjectList, spSpawnObjectList);

class SpawnObjectList : public oxygine::ref_counter
{
public:
   int m_probabilitySum;
   std::vector<spSpawnObject> m_spawnObjects;

   SpawnObjectList();

   void readSpawnObjectsNode(const pugi::xml_node& node);
   pugi::xml_node* getSpawnObjectNode(void);

};



DECLARE_SMART(SpawnInstruction, spSpawnInstruction);

class SpawnInstruction : public oxygine::ref_counter
{
public:
   int m_numOfSpawns;
   oxygine::Vector2 m_leftTop;
   oxygine::Vector2 m_widthHeight;
   spSpawnObjectList m_spawnSource;

   SpawnInstruction(
      int numOfSpawns, 
      oxygine::Vector2 leftTop, 
      oxygine::Vector2 widthHeight, 
      spSpawnObjectList spawnSource);


};