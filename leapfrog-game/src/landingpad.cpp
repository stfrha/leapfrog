
#include "landingpad.h"
#include "landingpadevents.h"

#include "sceneactor.h"

#include "gamestatus.h"

#include "actoruserdata.h"
#include "headdowndisplay.h"

using namespace oxygine;
using namespace std;


LandingPad::LandingPad(
   Resources& gameResources,
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
	initCompoundObjectParts(gameResources, sceneParent, sceneParent, parentObject, world, pos, root, string(""), groupIndex);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   // Here we attach Launch Site object to tree so it gets updates etc.
   attachTo(sceneParent);


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

         logs::messageln("Tick: %d", m_stableTicks);


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

         logs::messageln("Tick: %d", m_stableTicks);

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
         if ((m_latestLeapfrog->m_gameStatus->getFuel() < 100.0f) && (m_latestLeapfrog->m_gameStatus->getCredits() > 0))
         {
            m_latestLeapfrog->m_gameStatus->deltaFuel(0.2f);
            m_latestLeapfrog->m_gameStatus->deltaCredits(-1);
         }
      }


      break;
   }
}

void LandingPad::registerToMap(void)
{
   g_HeadDownDisplay->addMeToMap(
      MapItem::MapItemTypeEnum::neutralStationary, 
      getActor("landingPad1"), 
      MapItem::MapItemStateEnum::hollow);
}

