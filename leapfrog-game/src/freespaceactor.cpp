#include <algorithm>

#include "steerableobject.h"
#include "deepspacesceneevents.h"

#include "freespaceactor.h"
#include "bodyuserdata.h"


using namespace oxygine;
using namespace std;
using namespace pugi;

FreeSpaceActor::FreeSpaceActor(
   Resources& gameResources,
   b2World* world,
   xml_node& root,
   const string& initialState,
   int groupIndex) :
   SceneActor(gameResources, world, 0.4f),
   m_inOrbitField(false),
   m_state(insertBurn),
   m_stateChangeTime(0)
{
   m_sceneType = deepSpace;

   setPanorateMode(center);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, NULL, world, Vector2(0.0f, 0.0f), root, initialState, groupIndex);

   // Create background before the leapfrog
   generateBackground(gameResources);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
//   setPanorateObject(getObject("hammer1"));


   if (m_leapfrog != NULL)
   {
      m_leapfrogBody = m_leapfrog->getBody("lfMainBody");
      //   m_leapfrogBody = getBody("leapfrog1.lfMainBody");

      m_leapfrog->goToEnvironment(ENV_DEEP_SPACE);

      addBoundingBody(m_leapfrogBody);
   }

   m_lowerBoundary = new SoftBoundary(gameResources, m_world, 
      RectF(
         m_sceneWidth / 2.0f, 
         m_sceneHeight + 75.0f, 
         m_sceneWidth + 300.0f,
         150.0f), SoftBoundary::up);
   addChild(m_lowerBoundary);

   m_topBoundary = new SoftBoundary(gameResources, m_world, 
      RectF(
         m_sceneWidth / 2.0f,
         -75.0f, 
         m_sceneWidth + 300.0f,
         150.0f), SoftBoundary::down);
   addChild(m_topBoundary);

   m_leftBoundary = new SoftBoundary(gameResources, m_world, 
      RectF(
         -75.0f, 
         m_sceneHeight / 2.0f, 
         m_sceneHeight + 300.0f,
         150.0f), SoftBoundary::right);
   addChild(m_leftBoundary);

   m_rightBoundary = new SoftBoundary(gameResources, m_world, 
      RectF(
         m_sceneWidth + 75.0f, 
         m_sceneHeight / 2.0f,
         m_sceneHeight + 300.0f,
         150.0f), SoftBoundary::left);
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