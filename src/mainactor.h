#pragma once

#include "oxygine-framework.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitscene.h"
#include "compoundobject.h"

enum SceneTypeEnum
{
   STE_LANDING,
   STE_FREE_SPACE,
   STE_ORBIT
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

   spBox2DDraw m_debugDraw;

   CompoundObject* m_sceneObject;

   void transitToDeepSpaceListner(oxygine::Event *ev);
   void transitToOrbitListner(oxygine::Event *ev);
   void landingCompleteListner(oxygine::Event *ev);


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


};

