#include "landingactor.h"

#include "marker.h"
#include "launchsite.h"
#include "compoundobject.h"
#include "layout.h"
#include "leapfrogevents.h"
#include "launchsiteevents.h"


using namespace oxygine;
using namespace std;
using namespace pugi;

LandingActor::LandingActor(
   b2World* world,
   float initialZoom,
   float zoom,
   xml_node& root,
   int groupIndex) :
   SceneActor(world, initialZoom, zoom)
{

   m_contactListener.InitContactListner(this);

   m_sceneType = landing;

   setPanorateMode(center);

   //   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   m_world->SetContactListener(&m_contactListener);

   initCompoundObjectParts(this, this, NULL, world, Vector2(0.0f, 0.0f), root, groupIndex, true);

}

void LandingActor::runFadeOut(void)
{
   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(Vector2(0.0f, 0.0f));
   fader->setSize(g_Layout.getStageSize());
   fader->setAlpha(255);
   fader->setPriority(255);
   fader->attachTo(this);
   spTween tween = fader->addTween(Actor::TweenAlpha(255), 1000);

   tween->setDoneCallback(CLOSURE(this, &LandingActor::fadeOutComplete));
   tween->detachWhenDone();
}



void LandingActor::transitToDeepSpace(oxygine::Event *ev)
{
   runFadeOut();
}

void LandingActor::fadeOutComplete(Event *ev)
{
   ExitLandingSceneEvent event(1);
   dispatchEvent(&event);
}


void LandingActor::startLeapfrogInScene(string name)
{
   // Must run base class implementation first
   SceneActor::startLeapfrogInScene(name);

   if (m_leapfrog != NULL)
   {
      m_leapfrog->goToEnvironment(ENV_GROUND);
   }
}


void LandingActor::startLaunchSiteInScene(string name)
{
   LaunchSite* ls = static_cast<LaunchSite*>(getObject(name));

   if (ls != NULL)
   {
      ls->addEventListener(LaunchSequenceCompleteEvent::EVENT, CLOSURE(this, &LandingActor::transitToDeepSpace));
   }
}