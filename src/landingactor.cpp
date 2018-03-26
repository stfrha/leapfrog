#include "landingactor.h"

#include "static.h"
#include "marker.h"

using namespace oxygine;

LandingActor::LandingActor(Resources& gameResources) :
	SceneActor(gameResources)
{
   setPanorateMode(PME_CENTER);

//   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   //spMarker marker = new Marker(gameResources, RectF(500, 250, 100, 100));
   //addChild(marker);

   createLeapFrog(gameResources);

   m_leapfrog->goToMode(LFM_LANDING);
   m_leapfrog->goToEnvironment(ENV_GROUND);

   // I should probably load resources that are uniuqe to the landing mode here

	spStatic ground = new Static(gameResources, m_world, RectF(465, 270, 25, 1));
	addChild(ground);

	spStatic platform = new Static(gameResources, m_world, RectF(520, 230, 25, 1));
	addChild(platform);

	spStatic pillar = new Static(gameResources, m_world, RectF(500, 305, 3, 100));
   addChild(pillar);

}
