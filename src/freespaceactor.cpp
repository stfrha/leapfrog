#include "freespaceactor.h"
#include "asteroid.h"


using namespace oxygine;

FreeSpaceActor::FreeSpaceActor(Resources& gameResources) :
	SceneActor(gameResources)
{
   // I should probably load resources that are uniuqe to the free space mode here

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_leapfrog->goToMode(LFM_DEEP_SPACE);

   spAsteroid asteroid1 = new Asteroid(gameResources, m_world, b2Vec2(550.0f, 250.0f), ASE_SMALL);
   addChild(asteroid1);

   spAsteroid asteroid2 = new Asteroid(gameResources, m_world, b2Vec2(450.0f, 250.0f), ASE_MIDDLE);
   addChild(asteroid2);

   spAsteroid asteroid3 = new Asteroid(gameResources, m_world, b2Vec2(500.0f, 230.0f), ASE_LARGE);
   addChild(asteroid3);


}


