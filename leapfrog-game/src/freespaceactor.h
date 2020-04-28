#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"
#include "freespacecontactlistener.h"

DECLARE_SMART(FreeSpaceActor, spFreeSpaceActor);

class FreeSpaceActor : public SceneActor
{
public:
   enum States
   {
      firstUpdate,
      waitForLeapfrog,
      insertBurn,
      operate
   };


private:

   States m_state;
   int m_stateChangeTime;

   bool m_inOrbitField;
   oxygine::timeMS m_enteredOrbitFieldAtTime;

   FreeSpaceContactListener   m_contactListener;

   void runFadeIn(void);

public:
   b2Body * m_leapfrogBody;

   FreeSpaceActor(
      Resources& gameResources,
      b2World* world,
      float zoom,
      pugi::xml_node& root,
      int groupIndex);


   void startLeapfrogInScene(std::string name);

protected:
   void doUpdate(const oxygine::UpdateState &us);
   void generateBackground(Resources& gameResources);

};
