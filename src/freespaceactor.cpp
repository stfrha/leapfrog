#include <algorithm>

#include "asteroidfield.h"
#include "deepspacesceneevents.h"

#include "freespaceactor.h"


using namespace oxygine;

FreeSpaceActor::FreeSpaceActor(
   Resources& gameResources,
   std::string& fileName,
   std::string& initialState) :
	SceneActor(gameResources, 0.4f),
   m_inOrbitField(false),
   m_state(insertBurn),
   m_stateChangeTime(0)
{
   m_sceneType = STE_FREE_SPACE;

   setPanorateMode(PME_CENTER);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   readDefinitionXmlFile(gameResources, this, NULL, m_world, Vector2(0.0f, 0.0f), fileName, initialState);

   // Create background before the leapfrog
   generateBackground(gameResources);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
   m_leapfrog->goToEnvironment(ENV_DEEP_SPACE);

   m_leapfrog->setBoundedWallsActor(this);

   CompoundObject* co = m_leapfrog->getObject("lfMainBody");
   m_leapfrogBody = (b2Body*)co->getUserData();

   addBoundingBody(m_leapfrogBody);

   m_lowerBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, 575.0f, 1300.0f, 150.0f), RDE_UP);
   addChild(m_lowerBoundary);

   m_topBoundary = new SoftBoundary(gameResources, m_world, RectF(500.0f, -75.0f, 1300.0f, 150.0f), RDE_DOWN);
   addChild(m_topBoundary);

   m_leftBoundary = new SoftBoundary(gameResources, m_world, RectF(-75.0f, 250.0f, 800.0f, 150.0f), RDE_RIGHT);
   addChild(m_leftBoundary);

   m_rightBoundary = new SoftBoundary(gameResources, m_world, RectF(1075.0f, 250.0f, 800.0f, 150.0f), RDE_LEFT);
   addChild(m_rightBoundary);
}

CollisionEntityTypeEnum FreeSpaceActor::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
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

bool FreeSpaceActor::isInsideOrbitField(b2Body* body)
{
   return (m_lowerBoundary->isInside(body) && m_rightBoundary->isInside(body));

}

void FreeSpaceActor::doUpdate(const oxygine::UpdateState &us)
{
   SceneActor::doUpdate(us);

   testForBoundaryRepel();

   if (m_stateChangeTime == 0)
   {
      m_stateChangeTime = us.time;
      m_leapfrog->fireMainBooster(true);
      takeControlOfLeapfrog(true);
   }
   else
   {
      switch (m_state)
      {
      case insertBurn:
         if (us.time > m_stateChangeTime + 1000)
         {
            m_state = operate;
            m_stateChangeTime = us.time;
            m_leapfrog->fireMainBooster(false);
            takeControlOfLeapfrog(false);
         }

         break;
      case operate:
         // Do nothing special here
         break;
      }
   }


   if (m_inOrbitField)
   {
      if (isInsideOrbitField(m_leapfrogBody))
      {
         if (us.time > m_enteredOrbitFieldAtTime + 2500)
         {
            // Go to Orbit scene
            DeepSpaceSceneTranstToOrbitEvent event;
            dispatchEvent(&event);
         }
      }
      else
      {
         m_inOrbitField = false;
         m_enteredOrbitFieldAtTime = us.time;
      }
   }
   else
   {
      if (isInsideOrbitField(m_leapfrogBody))
      {
         m_inOrbitField = true;
         m_enteredOrbitFieldAtTime = us.time;
      }
   }
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