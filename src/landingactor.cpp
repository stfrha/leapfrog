#include "landingactor.h"

#include "static.h"
//#include "staticpolygon.h"
#include "marker.h"
#include "launchsite.h"
#include "compoundobject.h"
#include "leapfrogevents.h"
#include "launchsiteevents.h"

using namespace oxygine;
using namespace std;

LandingActor::LandingActor(
   Resources& gameResources, 
   string& fileName, 
   string& initialState) :
	SceneActor(gameResources)
{
   setPanorateMode(PME_CENTER);

//   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObject(gameResources, this, NULL, m_world, Vector2(435.52f, 375.0f), fileName, initialState);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
   m_leapfrog->goToEnvironment(ENV_GROUND);

   m_leapfrog->addEventListener(LeapfrogModeReachedEvent::EVENT, CLOSURE(this, &LandingActor::modeReachedListener));
   
   // m_leapfrog->addEventListener(ObjectPropertyTriggeredEvent::EVENT, CLOSURE(this, &LandingActor::handlePropertyTriggeredEvent));
   // m_leapfrog->m_properties[LeapFrog::propXPos].registerPropertyEventTrigger(1, PropertyEventTrigger::insideRange, 1200.0f, 10000.0f);

   m_leapfrog->goToMode(LFM_LANDING);

   m_launchSite = static_cast<LaunchSite*>(getObject("launchSite1"));
   m_launchSite->addEventListener(LaunchSiteLeapfrogLandedEvent::EVENT, CLOSURE(this, &LandingActor::leapfrogLandedOnLaunchSiteHandler));
}

CollisionEntityTypeEnum LandingActor::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void LandingActor::modeReachedListener(Event *ev)
{
   LeapfrogModeReachedEvent* event = static_cast<LeapfrogModeReachedEvent*>(ev);
   logs::messageln("Mode reached with info: %s", event->extraInfo.c_str());

   float mode = m_leapfrog->m_properties[LeapFrog::propMode].getProperty();
   float state = m_leapfrog->m_properties[LeapFrog::propState].getProperty();
   logs::messageln("Mode is now: %d", (int)mode);
   logs::messageln("State is now: %d", (int)state);
}

void LandingActor::leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev)
{
   logs::messageln("Launch Site landing stable");

   m_launchSite->startLaunchSequence();
}

void LandingActor::handlePropertyTriggeredEvent(oxygine::Event *ev)
{
   ObjectPropertyTriggeredEvent* event = static_cast<ObjectPropertyTriggeredEvent*>(ev);

   // Decode the triggered event id:
   switch (event->m_triggedEventId)
   {
   case 1:
      logs::message("Entered right area, changeing mode.");
      m_leapfrog->m_properties[LeapFrog::propMode].extSetProperty((float)LFM_DEEP_SPACE);
      
      // Unregister this event so that it does not spam
      m_leapfrog->m_properties[LeapFrog::propXPos].unregisterPropertyEventTrigger(1);

      // Register new event for when we go back to the left
      m_leapfrog->m_properties[LeapFrog::propXPos].registerPropertyEventTrigger(2, PropertyEventTrigger::insideRange, -10000.0f, 1200.0f);

      break;
   case 2:
      logs::message("Entered left area, changeing mode.");
      m_leapfrog->m_properties[LeapFrog::propMode].extSetProperty((float)LFM_LANDING);

      // Unregister this event so that it does not spam
      m_leapfrog->m_properties[LeapFrog::propXPos].unregisterPropertyEventTrigger(2);

      // Register new event for when we go back to the left
      m_leapfrog->m_properties[LeapFrog::propXPos].registerPropertyEventTrigger(1, PropertyEventTrigger::insideRange, 1200.0f, 10000.0f);

      break;
   }

}




