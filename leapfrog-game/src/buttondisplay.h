#pragma once

#include "oxygine-framework.h"

#include "sceneactor.h"

DECLARE_SMART(ButtonDisplay, spButtonDisplay);

class ButtonDisplay : public oxygine::Actor
{
private:
   SceneActor* m_sceneActor;

   oxygine::Sprite* m_zoomInSprite;
   oxygine::Sprite* m_zoomOutSprite;
   oxygine::Sprite* m_manPanEnableSprite;
   oxygine::Sprite* m_manPanSprite;
   oxygine::Sprite* m_reloadSprite;
   oxygine::Sprite* m_turnLeftSprite;
   oxygine::Sprite* m_turnRightSprite;
   oxygine::Sprite* m_yesSprite;
   oxygine::Sprite* m_noSprite;
   oxygine::Sprite* m_boosterSprite;
   oxygine::Sprite* m_fireSprite;
   bool m_questionShown;

   // False in scenes without the camera pan button (orbit)
   bool m_panAvailable;

   oxygine::RectF m_turnRightButtonRect;
   oxygine::RectF m_turnLeftButtonRect;
   oxygine::RectF m_boosterButtonRect;
   oxygine::RectF m_fireButtonRect;
   oxygine::RectF m_pauseButtonRect;
   oxygine::RectF m_zoomInButtonRect;
   oxygine::RectF m_zoomOutButtonRect;
   oxygine::RectF m_panButtonRect;
   oxygine::RectF m_manPanEnableButtonRect;
   oxygine::RectF m_reloadButtonRect;

   int m_turnRightTouchIndex;
   int m_turnLeftTouchIndex;
   int m_boosterTouchIndex;
   int m_fireTouchIndex;
   int m_pauseTouchIndex;
   int m_zoomInTouchIndex;
   int m_zoomOutTouchIndex;
   int m_panTouchIndex;
   int m_manPanEnableTouchIndex;
   int m_reloadTouchIndex;

   // This class is defined in, and used by, sceneactor.h but the only
   // instance of this is owned by the main actor, to maintain position
   // and manual pan mode between scenes (or after reloading of scenes)
   ManualPan m_manualPan;

   oxygine::Vector2 m_panStartPos;

   void setManualPanButtonState(void);
   void createButtonOverlay(void);


public:

   ButtonDisplay();

   void initialiseButtonDisplay(
      SceneActor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   // The camera pan button (and the zoom, pan and reload buttons it
   // enables) shown and usable, or hidden and ignored
   void setPanButtonAvailable(bool available);

   void sceneDownHandler(oxygine::Event* event);
   void sceneUpHandler(oxygine::Event* event);
   void panMoveHandler(oxygine::Event* event);
   void sceneMoveHandler(oxygine::Event* event);

protected:
	void doUpdate(const oxygine::UpdateState& us) override;
};

