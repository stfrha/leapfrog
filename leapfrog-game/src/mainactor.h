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
	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
	oxygine::Resources m_gameResources;
   oxygine::Resources m_splashScreenResource;

   b2World* m_world;

   spBox2DDraw m_debugDraw;

   SceneActor* m_sceneObject;

   spSprite m_zoomInSprite;
   spSprite m_zoomOutSprite;
   spSprite m_manPanEnableSprite;
   spSprite m_manPanSprite;
   spSprite m_reloadSprite;

   oxygine::RectF m_turnRightButtonRect;
   oxygine::RectF m_turnLeftButtonRect;
   oxygine::RectF m_boosterButtonRect;
   oxygine::RectF m_fireButtonRect;
   oxygine::RectF m_zoomInButtonRect;
   oxygine::RectF m_zoomOutButtonRect;
   oxygine::RectF m_panButtonRect;
   oxygine::RectF m_manPanEnableButtonRect;
   oxygine::RectF m_reloadButtonRect;

   // This class is defined in, and used by, sceneactor.h but the only
   // instance of this is owned by the main actor, to maintain position
   // and manual pan mode between scenes (or after reloading of scenes)
   ManualPan   m_manualPan;

   int m_turnRightTouchIndex;
   int m_turnLeftTouchIndex;
   int m_boosterTouchIndex;
   int m_fireTouchIndex;
   int m_zoomInTouchIndex;
   int m_zoomOutTouchIndex;
   int m_panTouchIndex;
   int m_manPanEnableTouchIndex;
   int m_reloadTouchIndex;
   oxygine::Vector2 m_panStartPos;

   bool m_reloadPressed;
   bool m_reloadArm;
   bool m_splashArm;

   //spObjectResources m_gameStatus;

   NextSceneDefinition m_nextScene;

   void initMainActor(void);
   void exitLandingScene(oxygine::Event *ev);
   void exitDeepSpaceScene(oxygine::Event *ev);
   void exitOrbitScene(oxygine::Event *ev);
   void exitHyperspaceScene(oxygine::Event *ev);
   void resourceDepletedHandler(oxygine::Event *ev);
   void setManualPanButtonState(void);
   void fetchInternetScene(void);
   
   void httpLoaded(oxygine::Event*);

   void sceneDownHandler(Event* event);
   void sceneUpHandler(Event* event);
   void panMoveHandler(Event* event);
   void sceneMoveHandler(Event* event);

   void dummyHandler(Event* event);

   void recursiveRemoveChildren(oxygine::spActor& parent);

public:
	MainActor();
	~MainActor();

   void registerLeapfrog(LeapFrog* leapfrog);

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

protected:
   void doUpdate(const UpdateState& us);
   void createButtonOverlay(void);

};

