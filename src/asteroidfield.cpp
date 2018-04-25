#include "freespaceactor.h"
#include "asteroid.h"
#include "asteroidfield.h"

using namespace oxygine;
using namespace pugi;

AsteroidField::AsteroidField(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   xml_node& objectNode) :
   m_gameResources(&gameResources),
   m_world(world),
   m_spaceActor((FreeSpaceActor*)sceneParent),
   m_timeSinceLast(0)
{
   readAsteroidFieldNode(objectNode);
   attachTo(sceneParent);
   spawnAsteroids();
}

void AsteroidField::readAsteroidFieldNode(xml_node& objectNode)
{
   m_leftTop = b2Vec2(objectNode.attribute("left").as_float(), objectNode.attribute("top").as_float());
   m_rightBottom = b2Vec2(objectNode.attribute("right").as_float(), objectNode.attribute("bottom").as_float());
   m_initialSpawn = objectNode.attribute("spawnInitial").as_int();
   m_interval = 1.0f / objectNode.attribute("intensity").as_float() * 1000.0f;
   m_fieldLifetime = objectNode.attribute("lifeTime").as_int();

   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_LARGE, m_leftTop, m_rightBottom ));
   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_MIDDLE, m_leftTop, m_rightBottom));
   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_SMALL, m_leftTop, m_rightBottom));

}

CollisionEntityTypeEnum AsteroidField::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}



void AsteroidField::addAsteroidSpawnInstruction(AsteroidSpawnInstruction& inst)
{
   m_asteroidSpawnList.push_back(inst);
}


void AsteroidField::doUpdate(const oxygine::UpdateState& us)
{
   m_timeSinceLast += us.dt;

   if (m_timeSinceLast >= m_interval)
   {
      float size = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.0f));

      if (size < 0.5f)
      {
         addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_SMALL, m_leftTop, m_rightBottom));
      }
      else if (size < 1.5f)
      {
         addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_MIDDLE, m_leftTop, m_rightBottom));
      }
      else
      {
         addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_LARGE, m_leftTop, m_rightBottom));
      }

      m_timeSinceLast = 0;
   }

   if (m_spaceActor)
   {
      spawnAsteroids();
   }
}

void AsteroidField::spawnAsteroids(void)
{
   for (auto it = m_asteroidSpawnList.begin(); it != m_asteroidSpawnList.end(); ++it)
   {
      for (int i = 0; i < it->m_num; i++)
      {
         // Randomise position within asteroid field
         b2Vec2 pos;

         pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.x  - it->m_leftTop.x))) + it->m_leftTop.x;
         pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.y - it->m_leftTop.y))) + it->m_leftTop.y;
         
         spAsteroid asteroid = 
            new Asteroid(
               *m_gameResources, 
               (SceneActor*)m_spaceActor,
               m_world, 
               pos, 
               it->m_state, 
               this);
         asteroid->attachTo(m_spaceActor);
         m_spaceActor->addBoundingBody((b2Body*)asteroid.get()->getUserData());
      }
   }

   m_asteroidSpawnList.clear();
}

