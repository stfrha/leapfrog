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

// The end of the launch: zoom in and fade the view to white over the
// same time as the deep space scene fades in from white (FreeSpaceActor::
// runFadeIn), ending at the zoom that scene starts at, so the change of
// scene happens behind a white screen.
void LandingActor::runFadeOut(void)
{
   const timeMS c_launchFadeTime = 2000;

   zoomInToStartZoom(c_launchFadeTime);

   // The fader covers the view port, not the stage: the Leapfrog may have
   // flown above the top of the stage by now. The scene's parent is the
   // view port window (MainActor), which isn't scaled or panned.
   spColorRectSprite fader = new ColorRectSprite();
   fader->setColor(Color::White);
   fader->setPosition(Vector2(0.0f, 0.0f));
   fader->setSize(g_Layout.getViewPortBounds());
   fader->setAlpha(0);
   fader->setPriority(255);
   fader->setTouchEnabled(false);

   Actor* window = getParent();
   if (window != NULL)
   {
      fader->attachTo(window);
   }
   else
   {
      fader->setSize(g_Layout.getStageSize());
      fader->attachTo(this);
   }

   spTween tween = fader->addTween(Actor::TweenAlpha(255), c_launchFadeTime);
   tween->setDoneCallback(CLOSURE(this, &LandingActor::fadeOutComplete));
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