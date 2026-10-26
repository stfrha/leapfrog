
#include "landingpad.h"
#include "landingpadevents.h"

#include "mainactor.h"
#include "sceneactor.h"

#include "gamestatus.h"

#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "headdowndisplay.h"
#include "landinglimits.h"

namespace
{
   // How much of the scene to show after landing: wide enough to see the
   // buildings around the pad (the silo is about 128 units wide)
   const float c_landedVisibleWidth = 220.0f;
}

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
   m_active(true),
   m_announceLanded(false),
   m_fuelService(fuelNone),
   m_fuelRate(12.0f),
   m_fuelPrice(5.0f),
   m_fuelSupply(100.0f),
   m_creditsOwed(0.0f),
   m_fuelMessageShown(false),
   m_landedSent(false),
   m_stableTicks(0),
   m_latestLeapfrog(NULL),
   m_world(world),
   m_padBody(NULL),
   m_leftTouchTime(0),
   m_rightTouchTime(0),
   m_leftTouchVelocity(0.0f, 0.0f),
   m_rightTouchVelocity(0.0f, 0.0f),
   m_armHold(true)
{
	initCompoundObjectParts(sceneParent, sceneParent, parentObject, world, pos, root, groupIndex);

   // Active unless the behaviour says otherwise; Lua can change it (property 2)
   m_active = root.child("behaviour").attribute("active").as_bool(true);

   m_properties.push_back(ObjectProperty(this, NULL, state, (float)m_state, true));
   m_properties.push_back(ObjectProperty(this, NULL, lfLanded, 0.0f, true));
   m_properties.push_back(ObjectProperty(this, NULL, active, m_active ? 1.0f : 0.0f));

   // Fuel service (see landingpad.h). No fuel unless the scene says so.
   pugi::xml_node behaviour = root.child("behaviour");
   string fuel = behaviour.attribute("fuel").as_string("none");
   m_fuelService = (fuel == "sell") ? fuelSell : ((fuel == "free") ? fuelFree : fuelNone);
   m_fuelRate = behaviour.attribute("fuelRate").as_float(12.0f);
   m_fuelPrice = behaviour.attribute("fuelPrice").as_float(5.0f);
   m_fuelSupply = behaviour.attribute("fuelSupply").as_float(100.0f);

   m_properties.push_back(ObjectProperty(this, NULL, fuelSupply, m_fuelSupply));
   m_properties.push_back(ObjectProperty(this, NULL, fuelService, (float)m_fuelService));

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

   // The pad's body is the one in the contact that belongs to this pad
   b2Body* bodyA = contact->GetFixtureA()->GetBody();
   b2Body* bodyB = contact->GetFixtureB()->GetBody();
   if (BodyUserData::getParentObjectOfType<LandingPad*>(bodyA->GetUserData()) == this)
   {
      m_padBody = bodyA;
   }
   else if (BodyUserData::getParentObjectOfType<LandingPad*>(bodyB->GetUserData()) == this)
   {
      m_padBody = bodyB;
   }

   // Remember when and how fast each foot came down. This is called during
   // the physics step, so the hold itself is made in doUpdate.
   timeMS now = getTimeMS();
   b2Vec2 velocity = leapfrog->getCraftVelocity();

   if (leftFoot)
   {
      m_leftFootContact = true;
      m_leftTouchTime = now;
      m_leftTouchVelocity = velocity;
   }
   else
   {
      m_rightFootContact = true;
      m_rightTouchTime = now;
      m_rightTouchVelocity = velocity;
   }
}

bool LandingPad::isSafeLandingVelocity(const b2Vec2& v)
{
   return (fabs(v.x) <= LandingLimits::c_maxHorizontalSpeed) &&
      (v.y <= LandingLimits::c_maxDescentSpeed);
}

// Box2D destroys the joints of a body that is destroyed (e.g. when the
// Leapfrog breaks up), so check that a joint still exists before using it.
bool LandingPad::jointExists(b2Joint* joint)
{
   for (b2Joint* j = m_world->GetJointList(); j != NULL; j = j->GetNext())
   {
      if (j == joint)
      {
         return true;
      }
   }
   return false;
}

void LandingPad::holdLeapfrog(void)
{
   if ((m_latestLeapfrog == NULL) || (m_padBody == NULL) || !m_holdJoints.empty())
   {
      return;
   }

   const char* parts[] = { "lfMainBody", "lfLeftFoot", "lfRightFoot" };

   for (const char* part : parts)
   {
      b2Body* body = m_latestLeapfrog->getBody(part);
      if (body == NULL)
      {
         continue;
      }

      // Stop it where it is: no bounce left in it
      body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
      body->SetAngularVelocity(0.0f);

      b2WeldJointDef def;
      def.Initialize(m_padBody, body, body->GetWorldCenter());
      // Keep the contacts between the feet and the pad: with false, Box2D
      // removes them, the pad sees both feet leave (take-off, leg joints
      // unlocked), and sees them land again when the hold is released.
      def.collideConnected = true;
      m_holdJoints.push_back(m_world->CreateJoint(&def));
   }

   logs::messageln("Landing pad %s: holding the Leapfrog", getName().c_str());

   // Zoom in on the landing site, keeping the buildings around it in view
   if (m_sceneActor != NULL)
   {
      m_sceneActor->zoomInToVisibleWidth(c_landedVisibleWidth, c_landedZoomMs);
   }
}

void LandingPad::releaseLeapfrog(void)
{
   for (b2Joint* joint : m_holdJoints)
   {
      if (jointExists(joint))
      {
         m_world->DestroyJoint(joint);
      }
   }
   m_holdJoints.clear();

   logs::messageln("Landing pad %s: released the Leapfrog", getName().c_str());

   // Taking off: back out to the scene's default zoom
   if (m_sceneActor != NULL)
   {
      m_sceneActor->zoomOutToDefault(c_takeOffZoomMs);
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
   // Hold the Leapfrog on the pad after a good landing, let go when it boosts
   if (!m_holdJoints.empty())
   {
      if ((m_latestLeapfrog == NULL) || m_latestLeapfrog->isMainBoosterFiring())
      {
         releaseLeapfrog();
         m_armHold = false;
      }
   }
   else if (m_leftFootContact && m_rightFootContact)
   {
      if (m_armHold && (m_latestLeapfrog != NULL))
      {
         timeMS apart = (m_leftTouchTime > m_rightTouchTime) ?
            m_leftTouchTime - m_rightTouchTime : m_rightTouchTime - m_leftTouchTime;

         if ((apart <= c_touchWindowMs) &&
            isSafeLandingVelocity(m_leftTouchVelocity) &&
            isSafeLandingVelocity(m_rightTouchVelocity) &&
            !m_latestLeapfrog->isMainBoosterFiring())
         {
            holdLeapfrog();
         }
         else
         {
            // Too hard, too uneven or under power: no hold until the feet
            // have left the pad again
            m_armHold = false;
         }
      }
   }
   else if (!m_leftFootContact && !m_rightFootContact)
   {
      m_armHold = true;
   }

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
            m_properties[state].setProperty((float)m_state);
            m_properties[lfLanded].setProperty(0.0f);
            m_landedSent = false;
            if (m_active)
            {
               LandingPadLeapfrogTakeOffEvent event;
               dispatchEvent(&event);
            }
         }
      }
      else if (m_leftFootContact && m_rightFootContact)
      {
         m_stableTicks++;

         //logs::messageln("Tick: %d", m_stableTicks);

         if (m_stableTicks > 15)
         {
            m_state = LandingPadState::stableLand;
            m_properties[state].setProperty((float)m_state);
            m_properties[lfLanded].setProperty(1.0f);
            m_announceLanded = false;

            // A foot that lifts for a moment (a bounce, a sagging leg)
            // makes the pad unstable and then landed again, but it is
            // still the same landing: LpLL only once until a take-off
            if (!m_landedSent)
            {
               m_fuelMessageShown = false;

               if (m_active)
               {
                  m_landedSent = true;
                  LandingPadLeapfrogLandedEvent event;
                  dispatchEvent(&event);
               }
            }
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
         m_properties[state].setProperty((float)m_state);
      }
      break;

   case LandingPadState::stableLand:

      if (!m_leftFootContact || !m_rightFootContact)
      {
         m_stableTicks = 0;
         m_state = LandingPadState::unstable;
         m_properties[state].setProperty((float)m_state);
         m_announceLanded = false;
      }
      else if (m_announceLanded)
      {
         // Activated (by Lua) while the Leapfrog stood here: act as if it
         // had just landed. Done here, not in setPropertyImpl, so the event
         // doesn't reach Lua from inside its own c_setObjectProperty call.
         m_announceLanded = false;
         m_landedSent = true;
         LandingPadLeapfrogLandedEvent event;
         dispatchEvent(&event);
      }
      else if (m_active)
      {
         if (!g_MainActor->isInPause())
         {
            // Perhaps add fuel to some class of properties that are not allowed to change
            // while in pause, instead of this type of if statements.
            refuel(us.dtf);
         }
      }


      break;
   }
}

void LandingPad::setPropertyImpl(int propId, float value)
{
   if (propId == active)
   {
      bool wasActive = m_active;
      m_active = (value != 0.0f);

      if (m_active && !wasActive && (m_state == LandingPadState::stableLand))
      {
         m_announceLanded = true;
      }
      else if (!m_active)
      {
         m_announceLanded = false;
      }

      logs::messageln("Landing pad %s: %s", getName().c_str(), m_active ? "active" : "inactive");
   }
   else if (propId == fuelSupply)
   {
      // Lua refilled (or emptied) the pad's supply
      m_fuelSupply = value;
      m_fuelMessageShown = false;
   }
   else if (propId == fuelService)
   {
      int v = (int)(value + 0.5f);
      m_fuelService = (v == fuelSell) ? fuelSell : ((v == fuelFree) ? fuelFree : fuelNone);
      m_fuelMessageShown = false;
   }
}

// Fuel flows into the Leapfrog while it stands on an active pad that has
// fuel: bought with credits (sell) or from the pad's supply (free)
void LandingPad::refuel(float dt)
{
   if ((m_fuelService == fuelNone) || (m_latestLeapfrog == NULL) || (m_latestLeapfrog->m_resources == NULL) ||
      (dt <= 0.0f))
   {
      return;
   }

   ObjectResources* res = m_latestLeapfrog->m_resources;
   float room = 100.0f - res->getFuel();
   if (room <= 0.0f)
   {
      return;
   }

   float amount = m_fuelRate * dt;
   if (amount > room)
   {
      amount = room;
   }

   if (m_fuelService == fuelFree)
   {
      if (m_fuelSupply >= 0.0f)
      {
         if (amount > m_fuelSupply)
         {
            amount = m_fuelSupply;
         }
         if (amount <= 0.0f)
         {
            if (!m_fuelMessageShown && g_HeadDownDisplay)
            {
               g_HeadDownDisplay->logMessage("Fuel supply empty");
               m_fuelMessageShown = true;
            }
            return;
         }
         m_fuelSupply -= amount;
         if (m_fuelSupply < 0.001f)
         {
            m_fuelSupply = 0.0f;
         }
         m_properties[fuelSupply].setProperty(m_fuelSupply);
      }
      res->deltaFuel(amount);
   }
   else
   {
      // sell: as much as the credits pay for, charging whole credits as
      // the cost adds up
      int credits = res->getCredits();
      if (m_fuelPrice > 0.0f)
      {
         float affordable = ((float)credits - m_creditsOwed) / m_fuelPrice;
         if (amount > affordable)
         {
            amount = affordable;
         }
      }
      if (amount <= 0.0f)
      {
         if (!m_fuelMessageShown && g_HeadDownDisplay)
         {
            g_HeadDownDisplay->logMessage("Not enough credits for fuel");
            m_fuelMessageShown = true;
         }
         return;
      }
      res->deltaFuel(amount);
      m_creditsOwed += amount * m_fuelPrice;
      int charge = (int)m_creditsOwed;
      if (charge > 0)
      {
         res->deltaCredits(-charge);
         m_creditsOwed -= (float)charge;
      }
   }
}
