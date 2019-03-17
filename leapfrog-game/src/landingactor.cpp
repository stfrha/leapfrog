#include "landingactor.h"

#include "marker.h"
#include "launchsite.h"
#include "compoundobject.h"
#include "leapfrogevents.h"
#include "launchsiteevents.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

LandingActor::LandingActor(
   Resources& gameResources,
   b2World* world,
   xml_node& root,
   const string& initialState,
   int groupIndex) :
   SceneActor(gameResources, world, 0.4f, initialState)
{

   m_contactListener.InitContactListner(this);

   m_sceneType = landing;

   setPanorateMode(center);

   //   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, NULL, world, Vector2(0.0f, 0.0f), root, initialState, groupIndex);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));

   if (m_leapfrog != NULL)
   {
      m_leapfrog->goToEnvironment(ENV_GROUND);
      m_leapfrog->goToMode(LFM_LANDING);
   }

   m_launchSite = static_cast<LaunchSite*>(getObject("launchSite1"));

   if (m_launchSite != NULL)
   {
      m_launchSite->addEventListener(LaunchSiteLeapfrogLandedEvent::EVENT, CLOSURE(this, &LandingActor::leapfrogLandedOnLaunchSiteHandler));
      m_launchSite->addEventListener(LaunchSequenceCompleteEvent::EVENT, CLOSURE(this, &LandingActor::transitToDeepSpace));
   }

}

void LandingActor::leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev)
{
   logs::messageln("Launch Site landing stable");

   m_launchSite->startLaunchSequence(m_leapfrog.get());
}

void LandingActor::transitToDeepSpace(oxygine::Event *ev)
{
   LandingActorTransitToDeepSpaceEvent event;
   dispatchEvent(&event);
}


