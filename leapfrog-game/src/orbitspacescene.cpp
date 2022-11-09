#include <algorithm>

#include "orbitspacesceneevents.h"

#include "orbitspacescene.h"
#include "actoruserdata.h"

#include "graphicresources.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

OrbitSpaceScene::OrbitSpaceScene(
   b2World* world,
   xml_node& root,
   int groupIndex) :
	SceneActor(world, 0.4f, 0.4f),
   m_state(waitForLeapfrog),
   m_stateStartTime(0)
{
   m_sceneType = orbit;

   setPanorateMode(fix);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

//   m_world->SetContactListener(&m_contactListener);

//   readDefinitionXmlFile(gameResources, this, NULL, m_world, Vector2(0.0f, 0.0f), fileName, initialState);
   initCompoundObjectParts(this, this, this, world, Vector2(0.0f, 0.0f), root, groupIndex, true);

   // Create background before the leapfrog
   // generateBackground(gameResources);


   spSprite background = new Sprite();
   background->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim("starfield"));
   background->setSize(800.0f, 300.0f);
   background->setPosition(-150.0f, 0.0f);
   background->setScale(0.3f);
   background->setPriority(26);
   addChild(background);


   spTween t = background->addTween(TweenPosition(0.0f, 0.0f), 6000, 1, false, 0, Tween::ease_outQuad);
   t->setDoneCallback(CLOSURE(this, &OrbitSpaceScene::atOrbitEstablished));

}

void OrbitSpaceScene::doUpdate(const oxygine::UpdateState &us)
{
   SceneActor::doUpdate(us);

   switch (m_state)
   {
   case waitForLeapfrog:
      
      if (m_leapfrog != NULL)
      {
         takeControlOfLeapfrog(true);
         m_stateStartTime = us.time;
         m_state = enteringOrbit;
         m_leapfrog->fireMainBooster(true);
      }
      break;

   case enteringOrbit:

      if (us.time >= m_stateStartTime + 4000)
      {
         m_stateStartTime = us.time;
         m_state = slowToStop;
      }
      break;
   case slowToStop:
      m_leapfrog->fireMainBooster(false);

      // The next state will be entered when the background
      // tween is complete.
      // By continuously storing the time here it will be
      // at the state change time when we enter next state
      m_stateStartTime = us.time;
      break;

   case turnBooster:

      if (us.time >= m_stateStartTime + 1000)
      {
         m_leapfrog->fireSteeringBooster(0);
         m_stateStartTime = us.time;
         m_state = goToModeReentry;
      }
      break;

   case goToModeReentry:
      m_leapfrog->goToMode(LFM_REENTRY);

      if (us.time >= m_stateStartTime + 5000)
      {
         OrbitSpaceOrbitEstablished event;
         dispatchEvent(&event);

         m_stateStartTime = us.time;
         m_state = orbitEstablished;
      }
      break;

   case orbitEstablished:

      break;
   }

}

void OrbitSpaceScene::atOrbitEstablished(Event* event)
{
   m_leapfrog->setHoldAngle(-MATH_PI/2.0f);
   m_leapfrog->fireSteeringBooster(-1);
   m_state = turnBooster;
}

void OrbitSpaceScene::startLeapfrogInScene(string name)
{
   // Must run base class implementation first
   SceneActor::startLeapfrogInScene(name);

   if (m_leapfrog != NULL)
   {
      m_leapfrog->goToEnvironment(ENV_ORBIT);
      m_leapfrog->setHoldAngle(MATH_PI / 2.0f);
      CompoundObject* co = m_leapfrog->getObject("lfMainBody");
      m_leapfrogBody = m_leapfrog->getBody("lfMainBody");
   }
}


