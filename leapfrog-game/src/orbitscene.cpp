#include <algorithm>

#include "orbitspacesceneevents.h"

#include "orbitspacescene.h"
#include "planetactor.h"
#include "leapfrog.h"
#include "orbitspacescene.h"
#include "orbitwindow.h"
#include "orbitscene.h"
#include "layout.h"

using namespace oxygine;
using namespace pugi;
using namespace std;

OrbitScene::OrbitScene(
   Resources& gameResources,
   CompoundObject* parentObject,
   b2World* world,
   xml_node& root,
   const std::string& initialState,
   int groupIndex) :
      CompoundObject(NULL, parentObject),
      m_state(enteringOrbit)
{
   // Nothing should be read here 
   // initCompoundObjectParts(gameResources, this, NULL, world, Vector2(0.02f, 0.0f), root, initialState, groupIndex);

   m_planet = new PlanetActor(gameResources, this, root.child("behaviour").child("properties"));

   // Here we need to calculate some geometrics
   // Position, size, scale and frameThickness of OrbitWindow is needed
   // Original values are:
   // posX 550.0f,
   // posY   0.0f,
   // Width   300.0f,
   // Height   150.0f,
   // Frame thickness   1.0f,
   // Scale   0.4f,

   float x = g_Layout.getXFromRight(2) - g_Layout.getButtonWidth() / 2.0f;
   float y = g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f;
   float w = g_Layout.getButtonWidth() * 3.0f;
   float h = g_Layout.getButtonWidth() * 1.5f;
   float s = 0.4f * w / 300.0f;
   float ft = 1.0f / 300.0f * w;

   // Below creates the space scene window incl the space scene itself
   spOrbitWindow orbWin = new OrbitWindow(
      gameResources,
      this,
      world,
      x,
      y,
      w,
      h,
      ft,
      s,
      m_planet->m_spaceSceneBackground,
      m_planet->m_spaceSceneFile,
      "default");

   m_space = static_cast<OrbitSpaceScene*>(orbWin->getObject("spaceScene"));

   m_leapfrog = static_cast<LeapFrog*>(orbWin->getObject("spaceScene.leapfrog1"));

//   m_leapfrog->initGameStatus(gameStatus);

   m_space->addEventListener(OrbitSpaceOrbitEstablished::EVENT, CLOSURE(this, &OrbitScene::orbitEstablishedHandler));

}

void OrbitScene::doUpdate(const oxygine::UpdateState &us)
{
   const Uint8* data = SDL_GetKeyboardState(0);
   int time;
   float burnAmount;
   int heat = 0;

   switch (m_state)
   {
   case enteringOrbit:
      break;
   case waitForUserBreak:
      
      if (data[SDL_SCANCODE_W] || m_space->m_boosterPressed)
      {
         m_planet->startBurn();
         m_leapfrog->fireMainBooster(true, true);
         m_state = breaking;
      }

      break;
   case breaking:
      // TODO: Add timeout for if burn is too long, it should fall regardless
      if (!data[SDL_SCANCODE_W] && !m_space->m_boosterPressed)
      {
         m_planet->stopBurn();
         m_stateChangeTime = us.time;
         m_state = reentry;
         m_leapfrog->fireMainBooster(false);
         m_leapfrog->fireReentryFlames(true);

         // Here the burn is complete
         float burnAmount = m_planet->getBurnAmount();

         m_planet->startReentry((0.7f - burnAmount) / 0.1f, 150.0f);
      }
      break;
   case reentry:
      time = us.time - m_stateChangeTime;
      burnAmount = m_planet->getBurnAmount();

      if (time > 10200)
      {
         heat = 0;
         m_leapfrog->reentrySetHeat(heat);
         m_planet->surfaceReached();
         m_stateChangeTime = us.time;
         m_state = touchDown;
         m_leapfrog->dumpReentryFlamees();
      }
      else
      {
         if (burnAmount > 0.8)
         {
            // Too much break burn, falls too fast, burns on reentry
            if (time < 5000)
            {
               m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

               if (time > 3333)
               {
                  // increse burn
                  int heat = (int)((time - 3333.0f) * 255.0f / 1666.0f);
                  m_leapfrog->reentrySetHeat(heat);
               }
            }
            else
            {
               // Dissassemble the leapfrog to pieces whith flames
               m_leapfrog->setHoldAngle(20.0f * MATH_PI);

               if (time > 9000)
               {
                  // Dissasmble leapfrog
                  m_leapfrog->breakJoints();
               }

            }
         }
         else if (burnAmount < 0.6)
         {
            // Too little break burn, going too fast, bounce off of atmosphere
            if (time < 5000)
            {
               m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

               if (time > 3333)
               {
                  // increse burn
                  heat = (int)((time - 3333.0f) * 255.0f / 3333.0f);
                  m_leapfrog->reentrySetHeat(heat);
               }
            }
            else
            {
               // Remove heat and tumble the leapfrog
               m_leapfrog->setHoldAngle(20.0f * MATH_PI);
               heat = 0;
               m_leapfrog->reentrySetHeat(heat);
            }

         }
         else
         {
            m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

            if (time > 9000)
            {
               // decrese burn
               heat = (10000 - time) * 255 / 1000;
               m_leapfrog->reentrySetHeat(heat);
            }
            else if (time > 3333)
            {
               // increse burn
               heat = (int)((time - 3333.0f) * 255.0f / 3333.0f);
               m_leapfrog->reentrySetHeat(heat);
            }
         }
      }

      break;
   case touchDown:
      
      // Sending landing complete event with the result
      OrbitSceneLandingComplete event(m_planet->getLandingResult());
      dispatchEvent(&event);
      break;
   }
}

void OrbitScene::orbitEstablishedHandler(oxygine::Event *ev)
{
   m_planet->orbitEstablished();
   m_state = waitForUserBreak;
}
