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
   float initialZoom,
   float zoom,
   xml_node& root,
   int groupIndex) :
   SceneActor(gameResources, world, initialZoom, zoom),
   m_inTransitField(0),
   m_state(firstUpdate),
   m_leapfrogBody(NULL),
   m_stateChangeTime(0)
{
   m_sceneType = deepSpace;

   setPanorateMode(center);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, this, NULL, world, Vector2(0.0f, 0.0f), root, groupIndex, true);

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
      if (m_inTransitField)
      {
         m_inTransitField = isInsideTransitField(m_leapfrogBody);
         if (m_inTransitField)
         {
            if (us.time > m_enteredTransiFieldAtTime + 2500)
            {
               // Go to Orbit scene
               ExitDeepSpaceSceneEvent event(m_inTransitField);
               dispatchEvent(&event);
            }
         }
         else
         {
            m_enteredTransiFieldAtTime = us.time;
         }
      }
      else
      {
         m_inTransitField = isInsideTransitField(m_leapfrogBody);
         if (m_inTransitField)
         {
            m_enteredTransiFieldAtTime = us.time;
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



