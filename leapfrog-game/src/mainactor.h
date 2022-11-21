#pragma once

#include "oxygine-framework.h"
#include "sceneactor.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitscene.h"
#include "sceneactor.h"
#include "gamestatus.h"

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

   enum PostMenuActionType
   {
      continueCurrent,
      continueLatest,
      startNew,
      testLanding,
      testSpace,
      testOrbit
   };

   PostMenuActionType m_postMenuAction;

	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
   oxygine::Resources m_splashScreenResource;

   b2World* m_world;

   spBox2DDraw m_debugDraw;

   SceneActor* m_sceneObject;

   NextSceneDefinition m_nextScene;

   bool m_reloadArm;
   bool m_splashArm;
   bool m_menuArm;

   void initMainActor(void);
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
   bool isInPause(void);
   void buttonClicked(int button);
   void restartedFromMenu(void);
   void menuStartTransitionComplete(void);
   void cleanUpAndQuit(void);

protected:
   void doUpdate(const UpdateState& us);

};

extern spMainActor g_MainActor;
