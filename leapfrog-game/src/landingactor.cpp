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
   float zoom,
   xml_node& root,
   int groupIndex) :
   SceneActor(gameResources, world, zoom)
{

   m_contactListener.InitContactListner(this);

   m_sceneType = landing;

   setPanorateMode(center);

   //   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(gameResources, this, this, NULL, world, Vector2(0.0f, 0.0f), root, groupIndex, true);

}

void LandingActor::leapfrogLandedOnLaunchSiteHandler(oxygine::Event *ev)
{
   logs::messageln("Launch Site landing stable");

   if (m_leapfrog != NULL)
   {
      LaunchSiteLeapfrogLandedEvent* lsev = (LaunchSiteLeapfrogLandedEvent*)ev;
      LaunchSite* sender = lsev->m_launchSite;
      sender->startLaunchSequence(m_leapfrog.get());
   }
}

void LandingActor::transitToDeepSpace(oxygine::Event *ev)
{
   LandingActorTransitToDeepSpaceEvent event;
   dispatchEvent(&event);
}


void LandingActor::startLeapfrogInScene(void)
{
   // Must run base class implementation first
   SceneActor::startLeapfrogInScene();

   if (m_leapfrog != NULL)
   {
      m_leapfrog->goToEnvironment(ENV_GROUND);
      m_leapfrog->goToMode(LFM_LANDING);
   }
}


// TODO: This should probably be more general. 
// For instance, to allow multiple Launch-sites
void LandingActor::startLaunchSiteInScene(string name)
{
   LaunchSite* ls = static_cast<LaunchSite*>(getObject(name));

   if (ls != NULL)
   {
      ls->addEventListener(LaunchSiteLeapfrogLandedEvent::EVENT, CLOSURE(this, &LandingActor::leapfrogLandedOnLaunchSiteHandler));
      ls->addEventListener(LaunchSequenceCompleteEvent::EVENT, CLOSURE(this, &LandingActor::transitToDeepSpace));
   }
}