#include "freespaceactor.h"
#include "asteroid.h"
#include "asteroidfield.h"
#include "sceneactor.h"

using namespace oxygine;
using namespace pugi;

AsteroidField::AsteroidField(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& objectNode) :
   CompoundObject(sceneParent),
   m_gameResources(&gameResources),
   m_world(world),
   m_sceneActor((SceneActor*)sceneParent),
   m_timeSinceLast(0)
{
   readAsteroidFieldNode(objectNode);
   attachTo(sceneParent);
   spawnAsteroids();
}

void AsteroidField::readAsteroidFieldNode(const xml_node& objectNode)
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

   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_LARGE, m_leftTop, m_rightBottom ));
   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_MIDDLE, m_leftTop, m_rightBottom));
   addAsteroidSpawnInstruction(AsteroidSpawnInstruction(m_initialSpawn / 3, ASE_SMALL, m_leftTop, m_rightBottom));

}

CollisionEntityTypeEnum AsteroidField::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}



void AsteroidField::addAsteroidSpawnInstruction(const AsteroidSpawnInstruction& inst)
{
   m_asteroidSpawnList.push_back(inst);
}


void AsteroidField::doUpdate(const oxygine::UpdateState& us)
{
   m_timeSinceLast += us.dt;

   if (m_timeSinceLast >= m_interval)
   {
      addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_AUTO, m_leftTop, m_rightBottom));

      m_timeSinceLast = 0;
   }

   if (m_sceneActor)
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
         Vector2 pos;

         pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.x  - it->m_leftTop.x))) + it->m_leftTop.x;
         pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.y - it->m_leftTop.y))) + it->m_leftTop.y;
         
         spAsteroid asteroid = 
            new Asteroid(
               *m_gameResources, 
               m_sceneActor,
               m_world, 
               pos, 
               it->m_state);
         asteroid->attachTo(m_sceneActor);

         if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
         {
            FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

            spaceActor->addBoundingBody((b2Body*)asteroid.get()->getUserData());
         }
      }
   }

   m_asteroidSpawnList.clear();
}

