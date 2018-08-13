#include <algorithm>

#include "orbitspacesceneevents.h"

#include "orbitspacescene.h"
#include "planetactor.h"
#include "leapfrog.h"
#include "orbitspacescene.h"

#include "orbitscene.h"

using namespace oxygine;

OrbitScene::OrbitScene(
   Resources& gameResources,
   const std::string& fileName,
   const std::string& initialState) :
   CompoundObject((SceneActor*)this),
   m_state(enteringOrbit)
{
   readDefinitionXmlFile(gameResources, (SceneActor*)this, NULL, NULL, Vector2(0.02f, 0.0f), fileName, initialState);

   m_space = static_cast<OrbitSpaceScene*>(getObject("orbitWindow.spaceScene"));

   // TODO: the string here will change for each planet, we need some
   // way of getting an object by type.
   m_planet = static_cast<PlanetActor*>(getObject("planetAlpha"));

   m_leapfrog = static_cast<LeapFrog*>(getObject("orbitWindow.spaceScene.leapfrog1"));

   m_space->addEventListener(OrbitSpaceOrbitEstablished::EVENT, CLOSURE(this, &OrbitScene::orbitEstablishedHandler));

}

CollisionEntityTypeEnum OrbitScene::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void OrbitScene::doUpdate(const oxygine::UpdateState &us)
{
   const Uint8* data = SDL_GetKeyboardState(0);
   int time;
   float burnAmount;

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

      if (burnAmount > 0.8)
      {
         // Too much break burn, falls too fast, burns on reentry
         if (time < 5000)
         {
            m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

            if (time > 3333)
            {
               // increse burn
               unsigned int heat = (unsigned int)((time - 3333.0f) * 255.0f / 1666.0f);

               if (heat > 255)
               {
                  heat = 255;
               }

               m_leapfrog->reentrySetHeat((unsigned char)heat);
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
               m_leapfrog->reentrySetHeat((unsigned int)((time - 3333.0f) * 255.0f / 3333.0f));
            }
         }
         else
         {
            // Remove heat and tumble the leapfrog
            m_leapfrog->setHoldAngle(20.0f * MATH_PI);
            m_leapfrog->reentrySetHeat(0);
         }

      }
      else
      {
         m_leapfrog->setHoldAngle(m_planet->getReentryLeapfrogAngle((float)time) - MATH_PI / 2.0f);

         // TODO: Model the reentry flames intensity as a heat-value from 0 (no flames) to 255 (max flame)
         // which now should be a function of the time since reentry began
         if (time > 9000)
         {
            // decrese burn
            unsigned char heat = (10000 - time) * 255 / 1000;
            m_leapfrog->reentrySetHeat(heat);
         }
         else if (time > 3333)
         {
            // increse burn
            unsigned int heat = (unsigned int)((time - 3333.0f) * 255.0f / 3333.0f);

            if (heat > 255)
            {
               heat = 255;
            }

            m_leapfrog->reentrySetHeat((unsigned char)heat);
         }
      }

      if (time > 10200)
      {
         m_leapfrog->reentrySetHeat(0xff);
         m_planet->surfaceReached();
         m_stateChangeTime = us.time;
         m_state = touchDown;
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
