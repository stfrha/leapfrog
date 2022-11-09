
#include "landingpad.h"
#include "landingpadevents.h"

#include "sceneactor.h"

#include "gamestatus.h"

#include "actoruserdata.h"
#include "headdowndisplay.h"

using namespace oxygine;
using namespace std;


LandingPad::LandingPad(
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_leftFootContact(false),
   m_rightFootContact(false),
   m_state(LandingPadState::unstable),
   m_stableTicks(0)
{
	initCompoundObjectParts(sceneParent, sceneParent, parentObject, world, pos, root, groupIndex);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   // Here we attach Launch Site object to tree so it gets updates etc.
   attachTo(sceneParent);


   // It is lame to use a fixed name for the sprite actor of the pad. 
   // It is also lame to assume that the child object only has one sprite.
   // The best solution is to have a proprty that points to the sprite actor
   // that holds the position. But, not for now. 
   // Now I use the first actor in the list.

   if (m_shapes.size() > 0)
   {
      spActor padActor = m_shapes[0];

      // Register leapfrog on map
      g_HeadDownDisplay->addMeToMap(
         MapItem::MapItemTypeEnum::neutralStationary,
         padActor,
         MapItem::MapItemStateEnum::hollow);
   }

}

void LandingPad::leapfrogFootTouch(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog)
{
   m_latestLeapfrog = leapfrog;

   if (leftFoot)
   {
      m_leftFootContact = true;
   }
   else
   {
      m_rightFootContact = true;
   }
}

void LandingPad::leapfrogFootLift(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog)
{
   m_latestLeapfrog = leapfrog;

   if (leftFoot)
   {
      m_leftFootContact = false;
   }
   else
   {
      m_rightFootContact = false;
   }
}

void LandingPad::doUpdate(const UpdateState &us)
{
   switch (m_state)
   {
   case LandingPadState::unstable:

      if (!m_leftFootContact && !m_rightFootContact)
      {
         m_stableTicks++;

         //logs::messageln("Tick: %d", m_stableTicks);


         if (m_stableTicks > 15)
         {
            m_state = LandingPadState::stableAir;
            LandingPadLeapfrogTakeOffEvent event;
            dispatchEvent(&event);
         }
      }
      else if (m_leftFootContact && m_rightFootContact)
      {
         m_stableTicks++;

         //logs::messageln("Tick: %d", m_stableTicks);

         if (m_stableTicks > 15)
         {
            m_state = LandingPadState::stableLand;
            LandingPadLeapfrogLandedEvent event;
            dispatchEvent(&event);
         }
      }
      else
      {
         m_stableTicks = 0;
      }
      break;

   case LandingPadState::stableAir:

      if (m_leftFootContact || m_rightFootContact)
      {
         m_stableTicks = 0;
         m_state = LandingPadState::unstable;
      }
      break;

   case LandingPadState::stableLand:

      if (!m_leftFootContact || !m_rightFootContact)
      {
         m_stableTicks = 0;
         m_state = LandingPadState::unstable;

      }
      else
      {
         if (!m_sceneActor->getIsInPause())
         {
            // Perhaps add fuel to some class of properties that are not allowed to change
            // while in pause, instead of this type of if statements.

            if ((m_latestLeapfrog->m_resources->getFuel() < 100.0f) && (m_latestLeapfrog->m_resources->getCredits() > 0))
            {
               m_latestLeapfrog->m_resources->deltaFuel(0.2f);
               m_latestLeapfrog->m_resources->deltaCredits(-1);
            }
         }
      }


      break;
   }
}

