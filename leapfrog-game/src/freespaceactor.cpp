#include <algorithm>

#include "freespaceactor.h"
#include "deepspacesceneevents.h"

#include "steerableobject.h"
#include "layout.h"

#include "bodyuserdata.h"


using namespace oxygine;
using namespace std;
using namespace pugi;

FreeSpaceActor::FreeSpaceActor(
   Resources& gameResources,
   b2World* world,
   float zoom,
   xml_node& root,
   int groupIndex) :
   SceneActor(gameResources, world, zoom),
   m_inOrbitField(false),
   m_state(firstUpdate),
   m_leapfrogBody(NULL),
   m_stateChangeTime(0)
{
   m_sceneType = deepSpace;

   setPanorateMode(center);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, this, NULL, world, Vector2(0.0f, 0.0f), root, groupIndex, true);

   // Create background before the leapfrog
   generateBackground(gameResources);



}

void FreeSpaceActor::doUpdate(const oxygine::UpdateState &us)
{
   SceneActor::doUpdate(us);

   // TODO: This is a "burn-into-space" animation, it should only run
   // if ascending from launch, So it must be executed on command.
   switch (m_state)
   {
   case firstUpdate:
      runFadeIn();
      m_state = waitForLeapfrog;
      break;
   case waitForLeapfrog:
      if (m_leapfrog != NULL)
      {
         m_stateChangeTime = us.time;
         m_leapfrog->fireMainBooster(true);
         takeControlOfLeapfrog(true);
         m_state = insertBurn;
      }
      break;
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

   if (m_leapfrogBody != NULL)
   {
      if (m_inOrbitField)
      {
         if (isInsideOrbitField(m_leapfrogBody))
         {
            if (us.time > m_enteredOrbitFieldAtTime + 2500)
            {
               // Go to Orbit scene
               ExitDeepSpaceSceneEvent event(3);
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
}

void FreeSpaceActor::runFadeIn(void)
{
   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(Vector2(0.0f, 0.0f));
   fader->setSize(g_Layout.getStageSize());
   fader->setAlpha(255);
   fader->setPriority(255);
   fader->attachTo(this);
   spTween tween = fader->addTween(Actor::TweenAlpha(0), 1000);
   tween->detachWhenDone();

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

void FreeSpaceActor::startLeapfrogInScene(string name)
{
   // Must run base class implementation first
   SceneActor::startLeapfrogInScene(name);

   if (m_leapfrog != NULL)
   {
      m_leapfrogBody = m_leapfrog->getBody("lfMainBody");
      m_leapfrog->goToEnvironment(ENV_DEEP_SPACE);
   }
}


