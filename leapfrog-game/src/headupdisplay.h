#pragma once

#include "oxygine-framework.h"
#include "leapfrog.h"
#include "buttondisplay.h"

class SceneActor;

// The HUD contains the buttons and the status bar.

DECLARE_SMART(HeadUpDisplay, spHeadUpDisplay);

class HeadUpDisplay : public oxygine::Actor
{
private:

   SceneActor* m_sceneActor;
   ButtonDisplay* m_buttonDisplay;
   oxygine::Actor* m_statusBarActor;

   // Cheats on phones: double tap the fuel bar or the credits
   oxygine::RectF m_fuelCheatRect;
   oxygine::RectF m_creditCheatRect;
   int m_lastCheatTap;                 // 0 none, 1 fuel, 2 credits
   oxygine::timeMS m_lastCheatTapTime;
   bool checkCheatTap(const oxygine::Vector2& pos);

public:

   HeadUpDisplay();

   void initialiseHeadUpDisplay(
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
	void doUpdate(const oxygine::UpdateState& us) override;
};

extern spHeadUpDisplay g_headUpDisplay;
