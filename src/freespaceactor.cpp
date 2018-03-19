#include "freespaceactor.h"
#include "asteroid.h"


using namespace oxygine;

FreeSpaceActor::FreeSpaceActor(Resources& gameResources) :
	SceneActor(gameResources)
{
   // I should probably load resources that are uniuqe to the free space mode here

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_leapfrog->goToMode(LFM_DEEP_SPACE);
   m_boundedBodies.push_back((b2Body*)m_leapfrog->getUserData());

   spAsteroid asteroid1 = new Asteroid(gameResources, m_world, b2Vec2(550.0f, 250.0f), ASE_SMALL);
   addChild(asteroid1);
   m_boundedBodies.push_back((b2Body*)asteroid1->getUserData());

   spAsteroid asteroid2 = new Asteroid(gameResources, m_world, b2Vec2(450.0f, 250.0f), ASE_MIDDLE);
   addChild(asteroid2);
   m_boundedBodies.push_back((b2Body*)asteroid2->getUserData());

   spAsteroid asteroid3 = new Asteroid(gameResources, m_world, b2Vec2(500.0f, 230.0f), ASE_LARGE);
   addChild(asteroid3);
   m_boundedBodies.push_back((b2Body*)asteroid3->getUserData());

   m_lowerBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, 575.0f, 1300.0f, 150.0f), RDE_UP);
   addChild(m_lowerBoundary);

   m_topBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, -75.0f, 1300.0f, 150.0f), RDE_DOWN);
   addChild(m_topBoundary);

   m_leftBoundary = new SoftBoundary(gameResources, m_world, RectF(-75.0f, 250.0f, 800.0f, 150.0f), RDE_RIGHT);
   addChild(m_leftBoundary);

   m_rightBoundary = new SoftBoundary(gameResources, m_world, RectF(1075.0f, 250.0f, 800.0f, 150.0f), RDE_LEFT);
   addChild(m_rightBoundary);
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
}


