#include <algorithm>

#include "orbitspacesceneevents.h"

#include "orbitspacescene.h"
#include "actoruserdata.h"


using namespace oxygine;
using namespace std;

OrbitSpaceScene::OrbitSpaceScene(
   Resources& gameResources,
   float zoomScale,
   const string& backgroundTexture,
   const string& fileName,
   const string& initialState) :
	SceneActor(gameResources, NULL, zoomScale),
   m_state(enteringOrbit),
   m_stateStartTime(0)
{
   m_sceneType = STE_ORBIT;

   setPanorateMode(PME_FIX);

   m_world->SetGravity(b2Vec2(0.0f, 0.0f));

//   m_world->SetContactListener(&m_contactListener);

   readDefinitionXmlFile(gameResources, this, NULL, m_world, Vector2(0.0f, 0.0f), fileName, initialState);

   // Create background before the leapfrog
   // generateBackground(gameResources);

   m_leapfrog = static_cast<LeapFrog*>(getObject("leapfrog1"));
   m_leapfrog->goToEnvironment(ENV_ORBIT);
   m_leapfrog->setHoldAngle(MATH_PI / 2.0f);

   takeControlOfLeapfrog(true);

   CompoundObject* co = m_leapfrog->getObject("lfMainBody");
   m_leapfrogBody = ActorUserData::getBody(co->getUserData());

   spSprite background = new Sprite();
   background->setResAnim(gameResources.getResAnim(backgroundTexture));
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

   // Below will be done one time at the first update
   // so we know the time at the start of the state machine
   // We perform some initial work that should only be done
   // one time
   if (m_stateStartTime == 0)
   {
      m_stateStartTime = us.time;
      m_leapfrog->fireMainBooster(true);
   }

   switch (m_state)
   {
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