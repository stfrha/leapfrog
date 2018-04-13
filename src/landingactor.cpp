#include "landingactor.h"

#include "static.h"
//#include "staticpolygon.h"
#include "marker.h"
#include "launchsite.h"
#include "compoundobject.h"

using namespace oxygine;
using namespace std;

LandingActor::LandingActor(Resources& gameResources, std::string fileName) :
	SceneActor(gameResources)
{
   setPanorateMode(PME_TOP);

//   m_world->SetGravity(b2Vec2(0, 1.62));
   m_world->SetGravity(b2Vec2(0, 3.0f));

   initCompoundObject(gameResources, this, m_world, Vector2(435.52f, 375.0f), fileName);

   m_leapfrog = static_cast<LeapFrog*>(getObject("lfMainBody"));
   m_leapfrog->initLeapfrog(LFM_LANDING, 0.0f);
   m_leapfrog->goToEnvironment(ENV_GROUND);


   //createLeapFrog(gameResources);

   //spMarker marker = new Marker(gameResources, RectF(300.0f, 250.0f, 128.0f, 128.0f), m_world);
   //addChild(marker);

   //for (int i = 0; i < 10; i++)
   //{
   //   spColorRectSprite sp1 = new ColorRectSprite();
   //   sp1->setAnchor(0.0f, 0.0f);
   //   sp1->setSize(10.0f, 10.0f);
   //   sp1->setColor(Color::Aqua);
   //   sp1->setPosition(i * 20.0f + 300.0f - 64.0f, 250.0f - 64.0f - 10.0f);
   //   addChild(sp1);
   //}



   // I should probably load resources that are uniuqe to the landing mode here

	//spStatic ground = new Static(gameResources, m_world, RectF(465, 270, 25, 1));
	//addChild(ground);

	//spStatic platform = new Static(gameResources, m_world, RectF(520, 230, 25, 1));
	//addChild(platform);

	//spStatic pillar = new Static(gameResources, m_world, RectF(500, 305, 3, 100));
 //  addChild(pillar);

   //spStaticPolygon grotto = new StaticPolygon(gameResources, "landing_map_1.xml", m_world);
   //addChild(grotto);


//   spLaunchSite launchSite = new LaunchSite(gameResources, (Actor*)this, m_world, Vector2(435.52f, 375.0f), string("launch_site.xml"));
//   spCompoundObject landingScene = new CompoundObject(gameResources, (Actor*)this, m_world, Vector2(435.52f, 375.0f), string("landing_scene.xml"));
}
