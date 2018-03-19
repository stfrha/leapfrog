#include "landingactor.h"

#include "static.h"

using namespace oxygine;

LandingActor::LandingActor(Resources& gameResources) :
	SceneActor(gameResources)
{
//   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   m_leapfrog->goToMode(LFM_LANDING);

   // I should probably load resources that are uniuqe to the landing mode here

	spStatic ground = new Static(gameResources, m_world, RectF(465, 270, 25, 1));
	addChild(ground);

	spStatic platform = new Static(gameResources, m_world, RectF(520, 230, 25, 1));
	addChild(platform);

	spStatic pillar = new Static(gameResources, m_world, RectF(500, 305, 3, 100));
	addChild(pillar);
}


