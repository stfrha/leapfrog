#pragma once

#include "oxygine-framework.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitscene.h"
#include "sceneactor.h"
#include "gamestatus.h"

//DECLARE_SMART is helper, it does forward declaration and declares intrusive_ptr typedef for your class
DECLARE_SMART(MainActor, spMainActor)

class MainActor : public oxygine::Actor
{
private:
	//it is our resources
	//in real project you would have more than one Resources declarations.
	//It is important on mobile devices with limited memory and you would load/unload them
	oxygine::Resources m_gameResources;

   spBox2DDraw m_debugDraw;

   SceneActor* m_sceneObject;

   oxygine::Rect m_turnRightButtonRect;
   oxygine::Rect m_turnLeftButtonRect;
   oxygine::Rect m_boosterButtonRect;
   oxygine::Rect m_fireButtonRect;
   oxygine::Rect m_zoomInButtonRect;
   oxygine::Rect m_zoomOutButtonRect;

   spGameStatus m_gameStatus;

   void transitToDeepSpaceListner(oxygine::Event *ev);
   void fadeFromLanding(void);
   void transitToOrbitListner(oxygine::Event *ev);
   void landingCompleteListner(oxygine::Event *ev);
   void goToDeepSpaceFader(Event *ev);
   void resourceDepletedHandler(oxygine::Event *ev);

   //void calculateButtonGeometrics(void);
   //void turnLeftButtonDownHandler(Event* event);
   //void turnLeftButtonUpHandler(Event* event);
   //void turnRightButtonDownHandler(Event* event);
   //void turnRightButtonUpHandler(Event* event);
   //void boosterButtonDownHandler(Event* event);
   //void boosterButtonUpHandler(Event* event);
   //void fireButtonDownHandler(Event* event);
   //void fireButtonUpHandler(Event* event);
   //void zoomInButtonDownHandler(Event* event);
   //void zoomInButtonUpHandler(Event* event);
   //void zoomOutButtonDownHandler(Event* event);
   //void zoomOutButtonUpHandler(Event* event);

   void sceneDownHandler(Event* event);
   void sceneUpHandler(Event* event);
   void sceneMoveHandler(Event* event);

   void dummyHandler(Event* event);

public:

	MainActor();
	~MainActor();

   // Below is the interface to the game manager (LUA script enginge)
   // "std::string object" are on the form "objectA.objectAsB.objectBsC" in the hierarcical 
   // CompoundObject structure

   // TODO: Enum to be replaced by file name of scene xml
   /*
      startScene reads xml file which builds the tree of CompoundObjects.
   */
   void startScene(SceneTypeEnum scene);

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
   void sendEvent(int eventId);

protected:
   void doUpdate(const UpdateState& us);
   void createButtonOverlay(void);

};

