#include "LanderContactListener.h"
#include "collisionentity.h"
#include "bullet.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "landingpad.h"


void LanderContactListener::ContactHandler(b2Contact* contact, bool begin)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();


   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;

   bool launchSiteLeftRest = false; // true means right leg rest
   bool leapfrogLeftFoot = false; // true means right foot

   if (eA == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = false;
   }

   if (eB == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = false;
   }

   if (eA == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = true;
   }

   if (eB == CET_LAUNCH_SITE_RIGHT_FOOT_REST)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      launchSite = (LaunchSite*)t->getParentObject();
      launchSiteLeftRest = true;
   }

   if (eA == CET_LF_LEFT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = false;
   }

   if (eB == CET_LF_LEFT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = false;
   }

   if (eA == CET_LF_RIGHT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = true;
   }

   if (eB == CET_LF_RIGHT_FOOT)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      leapfrog = (LeapFrog*)t->getParentObject();
      leapfrogLeftFoot = true;
   }

   if (eA == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_BULLET)
   {
      bullet = (Bullet*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_LANDING_PAD)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureA()->GetBody()->GetUserData());
      landingPad = (LandingPad*)t->getParentObject();
   }

   if (eB == CET_LANDING_PAD)
   {
      CompoundObject* t = (CompoundObject*)(contact->GetFixtureB()->GetBody()->GetUserData());
      landingPad = (LandingPad*)t->getParentObject();
   }


   if (launchSite && leapfrog && (leapfrogLeftFoot == launchSiteLeftRest))
   {
      if (begin)
      {
         launchSite->leapfrogFootTouch(contact, leapfrogLeftFoot);
      }
      else
      {
         launchSite->leapfrogFootLift(contact, leapfrogLeftFoot);
      }
   }

   if (landingPad && leapfrog)
   {
      if (begin)
      {
         landingPad->leapfrogFootTouch(contact, leapfrogLeftFoot);
      }
      else
      {
         landingPad->leapfrogFootLift(contact, leapfrogLeftFoot);
      }
   }
}

void LanderContactListener::BeginContact(b2Contact* contact)
{
   ContactHandler(contact, true);
}

void LanderContactListener::EndContact(b2Contact* contact)
{
   ContactHandler(contact, false);
}



