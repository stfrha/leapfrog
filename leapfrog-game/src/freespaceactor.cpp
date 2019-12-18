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
   SceneActor(gameResources, world, 0.4f, initialState),
   m_inOrbitField(false),
   m_state(insertBurn),
   m_stateChangeTime(0)
{
   m_sceneType = deepSpace;

   setPanorateMode(center);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, this, NULL, world, Vector2(0.0f, 0.0f), root, initialState, groupIndex);

   // Create background before the leapfrog
   generateBackground(gameResources);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
//   setPanorateObject(getObject("hammer1"));


   if (m_leapfrog != NULL)
   {
      m_leapfrogBody = m_leapfrog->getBody("lfMainBody");
      m_leapfrog->goToEnvironment(ENV_DEEP_SPACE);

   }


}

void FreeSpaceActor::doUpdate(const oxygine::UpdateState &us)
{
   SceneActor::doUpdate(us);

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
            DeepSpaceSceneTransitToOrbitEvent event;
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
   for (int x = 0; x < 6; x++)
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