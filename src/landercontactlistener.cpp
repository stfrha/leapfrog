#include "LanderContactListener.h"
#include "collisionentity.h"
#include "bullet.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "landingpad.h"

void LanderContactListener::BeginContact(b2Contact* contact)
{
   CollisionEntityTypeEnum eA = ((CollisionEntity *)contact->GetFixtureA()->GetUserData())->getEntityType();
   CollisionEntityTypeEnum eB = ((CollisionEntity *)contact->GetFixtureB()->GetUserData())->getEntityType();


   LeapFrog* leapfrog = NULL;
   Bullet* bullet = NULL;
   LaunchSite* launchSite = NULL;
   LandingPad* landingPad = NULL;

   if (eA == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      launchSite = (LaunchSite*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LAUNCH_SITE_LEFT_FOOT_REST)
   {
      launchSite = (LaunchSite*)contact->GetFixtureB()->GetBody()->GetUserData();
   }

   if (eA == CET_LF_LEFT_FOOT)
   {
      leapfrog = (LeapFrog*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LF_LEFT_FOOT)
   {
      leapfrog = (LeapFrog*)contact->GetFixtureB()->GetBody()->GetUserData();
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
      landingPad = (LandingPad*)contact->GetFixtureA()->GetBody()->GetUserData();
   }

   if (eB == CET_LANDING_PAD)
   {
      landingPad = (LandingPad*)contact->GetFixtureB()->GetBody()->GetUserData();
   }


   if (launchSite && leapfrog)
   {
      // Leapfrog foot toouched Launch site foot rest
      launchSite->leapfrogLanded(contact);
   }

   if (landingPad && leapfrog)
   {
      // Leapfrog foot toouched Launch site foot rest
      landingPad->leapfrogLanded(contact);

   }

}

void LanderContactListener::EndContact(b2Contact* contact)
{

}



