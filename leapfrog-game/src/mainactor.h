#pragma once

#include "oxygine-framework.h"
#include "sceneactor.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitscene.h"
#include "sceneactor.h"
#include "gamestatus.h"
#include "fpsestimator.h"

class NextSceneDefinition
{
public:
   bool m_armNextScene;
   std::string m_nextSceneFile;
   SceneActor::SceneTypeEnum m_nextSceneType;

   NextSceneDefinition();

};


//DECLARE_SMART is helper, it does forward declaration and declares intrusive_ptr typedef for your class
DECLARE_SMART(MainActor, spMainActor)

class MainActor : public oxygine::Actor
{
private:
   oxygine::spClock m_clock;
   bool m_isInPause;
   bool m_isInMessagePause;

   enum PostMenuActionType
   {
      continueCurrent,
      continueLatest,
      startNew,
      testStart         // the menu's test row: lua/test_starts.lua
   };

   PostMenuActionType m_postMenuAction;

   // Test starts: the one chosen in the menu (1..count, kept for the
   // session), and whether the scene being started is one (no initial save)
   int m_testStartIndex;
   bool m_startingTest;

	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
   oxygine::Resources m_splashScreenResource;

   b2World* m_world;

   spFpsEstimator m_fpsEst;

   spBox2DDraw m_debugDraw;

   SceneActor* m_sceneObject;

   NextSceneDefinition m_nextScene;

   bool m_fpsEstArm; 
   bool m_splashArm;
   bool m_initArm;
   bool m_reloadArm;
   bool m_menuArm;

   void showSplashScreen(void);
   void initMainActor(void);
   oxygine::spClock createSceneClock(void);
   void exitLandingScene(oxygine::Event *ev);
   void exitDeepSpaceScene(oxygine::Event *ev);
   void exitOrbitScene(oxygine::Event *ev);
   void exitHyperspaceScene(oxygine::Event *ev);
   void resourceDepletedHandler(oxygine::Event *ev);
   void fetchInternetScene(void);
   
   void httpLoaded(oxygine::Event*);

   void dummyHandler(Event* event);

   void recursiveRemoveChildren(oxygine::spActor& parent);

public:
   bool m_reloadPressed;
   float m_fps;      // measured at start-up, only logged (scenes update at Scales::c_updateStep)

	MainActor();
	~MainActor();

   // Below is the interface to the game manager (LUA script enginge)
   // "std::string object" are on the form "objectA.objectAsB.objectBsC" in the hierarcical 
   // CompoundObject structure

   /*
      startScene reads xml file which builds the tree of CompoundObjects.
   */
   void startScene(void);
   void armScene(std::string sceneName, int sceneType);

   float getProperty(std::string object, int propId);
   void setProperty(std::string object, int propId, float value);
   void registerPropertyTrigger(
      std::string object,
      int propId,
      int eventId,
      PropertyEventTrigger::TriggerType type,
      float lowerLimit,
      float upperLimit);
   void registerDualPropTrigger(
      std::string object,
      int propId,
      DualPropEventTrigger* trigger);
   void unregisterDualPropTrigger(
      std::string object,
      int eventId);

   void goToPause(void);

   // True during the menu pause and while the player reads the
   // enlarged message display (both freeze the game)
   bool isInPause(void);

   // Pause without the menu, for reading the message display
   bool canPauseForMessages(void);
   void setMessagePause(bool pause);
   void buttonClicked(int button);

   // The menu's test row
   void testStartClicked(int index);
   int getTestStartIndex(void) { return m_testStartIndex; }
   void setTestStartIndex(int index) { m_testStartIndex = index; }
   void restartedFromMenu(void);
   void menuStartTransitionComplete(void);
   void cleanUpAndQuit(void);

protected:
   void doUpdate(const UpdateState& us) override;

};

extern spMainActor g_MainActor;
