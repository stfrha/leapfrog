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

   static const int c_numCorners = 5;     // transit fields are 1 to 4

   int m_inTransitField;
   bool m_transitOpen[c_numCorners];
   std::string m_transitMessage[c_numCorners];
   int m_blockedCornerShown;              // its message has been shown
   oxygine::timeMS m_enteredTransiFieldAtTime;

   FreeSpaceContactListener   m_contactListener;

   void runFadeIn(void);

   // The transit field the body is in, 0 if none or if that corner is shut
   int openTransitField(b2Body* body);

public:
   b2Body * m_leapfrogBody;

   // Opens or shuts a corner's exit: 1 upper right, 2 upper left, 3 lower
   // right, 4 lower left. message goes to the HUD log when the Leapfrog
   // flies into the shut corner. All are open when a scene starts.
   void setTransitOpen(int corner, bool open, const std::string& message);

   FreeSpaceActor(
      b2World* world,
      float initialZoom,
      float zoom,
      pugi::xml_node& root,
      int groupIndex);


   void startLeapfrogInScene(std::string name);

protected:
   void doUpdate(const oxygine::UpdateState &us) override;
   
};
