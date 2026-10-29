#include <algorithm>

#include "orbitspacesceneevents.h"

#include "orbitspacescene.h"
#include "planetactor.h"
#include "leapfrog.h"
#include "orbitwindow.h"
#include "orbitscene.h"
#include "layout.h"
#include "headdowndisplay.h"
#include "luainterface.h"

using namespace oxygine;
using namespace pugi;
using namespace std;

OrbitScene::OrbitScene(
   CompoundObject* parentObject,
   b2World* world,
   xml_node& root,
   int groupIndex) :
      CompoundObject(NULL, parentObject),
      m_state(enteringOrbit)
{
   // Nothing should be read here 
   // initCompoundObjectParts(gameResources, this, NULL, world, Vector2(0.02f, 0.0f), root, initialState, groupIndex);

   m_planet = new PlanetActor(this, root.child("behaviour").child("properties"));

   // Here we need to calculate some geometrics
   // Position, size, scale and frameThickness of OrbitWindow is needed
   // Original values are:
   // posX 550.0f,
   // posY   0.0f,
   // Width   300.0f,
   // Height   150.0f,
   // Frame thickness   1.0f,
   // Scale   0.4f,

   // An inset in the top right corner of the orbit display, over the
   // sky above the trajectory (it used to be right of the display, in
   // the column the message display shared with it)
   RectF view = HeadDownDisplay::orbitSpaceViewRect();
   float x = view.getX();
   float y = view.getY();
   float w = view.getWidth();
   float h = view.getHeight();
   float s = 0.4f * w / 300.0f;
   float ft = 1.0f / 300.0f * w;

   // Below creates the space scene window incl the space scene itself
   spOrbitWindow orbWin = new OrbitWindow(
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

   // m_leapfrog = static_cast<LeapFrog*>(orbWin->getObject("spaceScene.leapfrog1"));

   //if (m_leapfrog == NULL)
   //{
   //   int a = 10;
   //}

//   m_leapfrog->initObjectResources(gameStatus);

   m_space->addEventListener(OrbitSpaceOrbitEstablished::EVENT, CLOSURE(this, &OrbitScene::orbitEstablishedHandler));

}

void OrbitScene::doUpdate(const oxygine::UpdateState &us)
{
   // We do nothing until leapfrog is created.
   if (m_space->m_leapfrog == NULL)
   {
      return;
   }

   const Uint8* data = SDL_GetKeyboardState(0);
   int time;
   float burnAmount;
   int heat = 0;

   switch (m_state)
   {
   case enteringOrbit:
      break;
   case waitForUserBreak:

      // No burn while the sites came by: round again (the scripts restart
      // the orbit, see scene_navigator.lua)
      if (m_planet->allSolutionsPassed())
      {
         ReentryResult result;
         result.m_outcome = ReentryResult::missed;
         g_LuaInterface.setLastReentry(result);
         logs::messageln("Re-entry: no burn, every site passed");

         m_state = exited;
         ExitOrbitSceneEvent event(-4);
         dispatchEvent(&event);
         break;
      }

      if (data[SDL_SCANCODE_W] || m_space->m_boosterPressed)
      {
         m_planet->startBurn();
         m_space->m_leapfrog->fireMainBooster(true, true);
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
         m_space->m_leapfrog->fireMainBooster(false);
         m_space->m_leapfrog->fireReentryFlames(true);

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
         m_space->m_leapfrog->reentrySetHeat(heat);
         m_planet->surfaceReached();
         m_stateChangeTime = us.time;
         m_state = touchDown;
         m_space->m_leapfrog->dumpReentryFlamees();
      }
      else
      {
         if (burnAmount > 0.8)
         {
            // Too much break burn, falls too fast, burns on reentry
            if (time < 5000)
            {
               m_space->m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

               if (time > 3333)
               {
                  // increse burn
                  int heat = (int)((time - 3333.0f) * 255.0f / 1666.0f);
                  m_space->m_leapfrog->reentrySetHeat(heat);
               }
            }
            else
            {
               // Dissassemble the leapfrog to pieces whith flames
               m_space->m_leapfrog->setHoldAngle(20.0f * MATH_PI);

               if (time > 9000)
               {
                  // Dissasmble leapfrog
                  m_space->m_leapfrog->breakJoints();
               }

            }
         }
         else if (burnAmount < 0.6)
         {
            // Too little break burn, going too fast, bounce off of atmosphere
            if (time < 5000)
            {
               m_space->m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

               if (time > 3333)
               {
                  // increse burn
                  heat = (int)((time - 3333.0f) * 255.0f / 3333.0f);
                  m_space->m_leapfrog->reentrySetHeat(heat);
               }
            }
            else
            {
               // Remove heat and tumble the leapfrog
               m_space->m_leapfrog->setHoldAngle(20.0f * MATH_PI);
               heat = 0;
               m_space->m_leapfrog->reentrySetHeat(heat);
            }

         }
         else
         {
            m_space->m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

            if (time > 9000)
            {
               // decrese burn
               heat = (10000 - time) * 255 / 1000;
               m_space->m_leapfrog->reentrySetHeat(heat);
            }
            else if (time > 3333)
            {
               // increse burn
               heat = (int)((time - 3333.0f) * 255.0f / 3333.0f);
               m_space->m_leapfrog->reentrySetHeat(heat);
            }
         }
      }

      break;
   case touchDown:
   {
      // The outcome, for the scripts (c_lastReentry), and the exit: the
      // closest site's index when down on the surface, on its marker or
      // off it; -2 burned up (too much braking); -3 bounced back to space
      // (too little); -4 (in waitForUserBreak) no burn while the sites
      // came by. The same burn limits as the reentry state above.
      ReentryResult result;
      result.m_burn = m_planet->getBurnAmount();
      int exitParameter = 0;

      if (result.m_burn > 0.8f)
      {
         result.m_outcome = ReentryResult::burnedUp;
         exitParameter = -2;
      }
      else if (result.m_burn < 0.6f)
      {
         result.m_outcome = ReentryResult::bounced;
         exitParameter = -3;
      }
      else
      {
         LandingResult landing = m_planet->getLandingResult();
         result.m_outcome = landing.m_landingSuccessfull ? ReentryResult::onSite : ReentryResult::offSite;
         result.m_siteIndex = landing.m_closestSiteIndex;
         result.m_siteName = landing.m_closestSite;
         result.m_siteScene = landing.m_closestSiteScene;
         result.m_distance = landing.m_landingSuccessfull ? 0.0f : landing.m_distanceFromClosestSite;
         exitParameter = landing.m_closestSiteIndex;
      }

      logs::messageln("Re-entry: outcome %d, site %d (%s), %.0f off, burn %.2f",
         (int)result.m_outcome, result.m_siteIndex, result.m_siteName.c_str(), result.m_distance, result.m_burn);

      g_LuaInterface.setLastReentry(result);

      // Once: the scene keeps updating until the next one starts
      m_state = exited;

      ExitOrbitSceneEvent event(exitParameter);
      dispatchEvent(&event);
      break;
   }
   case exited:
      break;
   }
}

void OrbitScene::orbitEstablishedHandler(oxygine::Event *ev)
{
   m_planet->orbitEstablished();
   m_state = waitForUserBreak;
}
