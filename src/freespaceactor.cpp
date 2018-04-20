#include <algorithm>

#include "asteroidfield.h"

#include "freespaceactor.h"


using namespace oxygine;

FreeSpaceActor::FreeSpaceActor(
   Resources& gameResources,
   std::string& fileName,
   std::string& initialState) :
	SceneActor(gameResources)
{
   setPanorateMode(PME_CENTER);

   m_contactListener.setSceneActor((SceneActor*)this);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObject(gameResources, this, m_world, Vector2(435.52f, 375.0f), fileName, initialState);

   // Create background before the leapfrog
   generateBackground(gameResources);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
   m_leapfrog->goToEnvironment(ENV_DEEP_SPACE);

   m_leapfrog->setBoundedWallsActor(this);

   CompoundObject* co = m_leapfrog->getObject("lfMainBody");
   b2Body* mainBody = (b2Body*)co->getUserData();

   addBoundingBody(mainBody);

   AsteroidField* asteroidField = (AsteroidField*)(getObject("asteroids1"));


   //addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_SMALL, b2Vec2(550.0f, 250.0f)));
   //addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_MIDDLE, b2Vec2(450.0f, 250.0f)));
   //addAsteroidSpawnInstruction(AsteroidSpawnInstruction(1, ASE_LARGE, b2Vec2(500.0f, 230.0f)));
   //addAsteroidSpawnInstruction(AsteroidSpawnInstruction(15, ASE_LARGE, b2Vec2(100.0f, 100.0f)));

   //spAsteroid asteroid1 = new Asteroid(gameResources, (SceneActor*)this, m_world, b2Vec2(550.0f, 250.0f), ASE_SMALL, this);
   //addChild(asteroid1);
   //addBoundingBody((Actor*)asteroid1.get());

   //spAsteroid asteroid2 = new Asteroid(gameResources, (SceneActor*)this, m_world, b2Vec2(450.0f, 250.0f), ASE_MIDDLE, this);
   //addChild(asteroid2);
   //addBoundingBody((Actor*)asteroid2.get());

   //spAsteroid asteroid3 = new Asteroid(gameResources, (SceneActor*)this, m_world, b2Vec2(500.0f, 230.0f), ASE_LARGE, this);
   //addChild(asteroid3);
   //addBoundingBody((Actor*)asteroid3.get());

   m_lowerBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, 575.0f, 1300.0f, 150.0f), RDE_UP);
   addChild(m_lowerBoundary);

   m_topBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, -75.0f, 1300.0f, 150.0f), RDE_DOWN);
   addChild(m_topBoundary);

   m_leftBoundary = new SoftBoundary(gameResources, m_world, RectF(-75.0f, 250.0f, 800.0f, 150.0f), RDE_RIGHT);
   addChild(m_leftBoundary);

   m_rightBoundary = new SoftBoundary(gameResources, m_world, RectF(1075.0f, 250.0f, 800.0f, 150.0f), RDE_LEFT);
   addChild(m_rightBoundary);
}

void FreeSpaceActor::addBoundingBody(b2Body* body)
{
   m_boundedBodies.push_back(body);
}

void FreeSpaceActor::removeBoundingBody(b2Body* body)
{
   m_boundedBodies.erase(std::remove(
      m_boundedBodies.begin(), 
      m_boundedBodies.end(), 
      body),
      m_boundedBodies.end());
}

void FreeSpaceActor::testForBoundaryRepel(void)
{
   for (auto it = m_boundedBodies.begin(); it != m_boundedBodies.end(); ++it)
   {
      m_lowerBoundary->testForRepel(*it);
      m_topBoundary->testForRepel(*it);
      m_leftBoundary->testForRepel(*it);
      m_rightBoundary->testForRepel(*it);
   }
}

void FreeSpaceActor::doUpdate(const oxygine::UpdateState &us)
{
   SceneActor::doUpdate(us);

   testForBoundaryRepel();

   // spawnAsteroids();
}

void FreeSpaceActor::generateBackground(Resources& gameResources)
{
   for (int x = 0; x < 5; x++)
   {
      for (int y = 0; y < 3; y++)
      {
         spSprite background = new Sprite();
         background->setResAnim(gameResources.getResAnim("starfield"));
         background->setPosition(-150.0f + 256.0f * x, -150.0f + 256.0f * y);
         background->setSize(512.0f, 512.0f);
         background->setScale(0.5f);
         background->setPriority(26);
         background->attachTo(this);
      }
   }

}