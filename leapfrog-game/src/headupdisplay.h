#pragma once

#include "oxygine-framework.h"
#include "leapfrog.h"
#include "buttondisplay.h"

class MainActor;
class SceneActor;

// The HUD contains the buttons and the status bar.

DECLARE_SMART(HeadUpDisplay, spHeadUpDisplay);

class HeadUpDisplay : public oxygine::Actor
{
private:

   MainActor* m_mainActor;
   SceneActor* m_sceneActor;
   ButtonDisplay* m_buttonDisplay;
   oxygine::Actor* m_statusBarActor;

public:

   HeadUpDisplay();

   void initialiseHeadUpDisplay(
      MainActor* mainActor,
      SceneActor* sceneActor,
      const oxygine::Vector2& topLeft,
      const oxygine::Vector2& bottomRight);

   void showStatusBars(bool show);
   void showButtons(bool show);
   void cleanAndRemove(void);

   void registerLeapfrog(LeapFrog* leapfrog, bool hideShield);

   void sceneDownHandler(oxygine::Event* event);
   void sceneUpHandler(oxygine::Event* event);
   void panMoveHandler(oxygine::Event* event);
   void sceneMoveHandler(oxygine::Event* event);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spHeadUpDisplay g_headUpDisplay;
