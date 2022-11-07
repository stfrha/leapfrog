#include "ButtonDisplay.h"

#include "mainactor.h"
#include "layout.h"

using namespace std;
using namespace oxygine;


ButtonDisplay::ButtonDisplay() :
   m_turnRightTouchIndex(0),
   m_turnLeftTouchIndex(0),
   m_boosterTouchIndex(0),
   m_fireTouchIndex(0),
   m_zoomInTouchIndex(0),
   m_zoomOutTouchIndex(0),
   m_panTouchIndex(0),
   m_reloadTouchIndex(0)
{
}

void ButtonDisplay::initialiseButtonDisplay(
   Resources* hudResources,
   MainActor* mainActor,
   SceneActor* sceneActor,
   const Vector2& topLeft,
   const Vector2& bottomRight)
{
   m_hudResources = hudResources;
   m_mainActor = mainActor;
   m_sceneActor = sceneActor;
   createButtonOverlay();

}

void ButtonDisplay::sceneDownHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;
   Vector2 v = te->localPosition;

   // logs::messageln("Down with index: %d", te->index);

   Point p = Point((int)v.x, (int)v.y);

   if (m_turnLeftButtonRect.pointIn(p))
   {
      m_sceneActor->m_turnLeftPressed = true;
      m_turnLeftTouchIndex = te->index;
   }
   else if (m_turnRightButtonRect.pointIn(p))
   {
      m_sceneActor->m_turnRightPressed = true;
      m_turnRightTouchIndex = te->index;
   }
   else if (m_boosterButtonRect.pointIn(p))
   {
      m_sceneActor->m_boosterPressed = true;
      m_boosterTouchIndex = te->index;
   }
   else if (m_fireButtonRect.pointIn(p))
   {
      m_sceneActor->m_firePressed = true;
      m_fireTouchIndex = te->index;
   }
   else if (m_pauseButtonRect.pointIn(p))
   {
      m_sceneActor->m_pausePressed = true;
      m_pauseTouchIndex = te->index;
   }
   else if (m_zoomInButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneActor->m_zoomInPressed = true;
         m_zoomInTouchIndex = te->index;
      }
   }
   else if (m_zoomOutButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneActor->m_zoomOutPressed = true;
         m_zoomOutTouchIndex = te->index;
      }
   }
   else if (m_manPanEnableButtonRect.pointIn(p))
   {
      m_sceneActor->m_manPanEnablePressed = true;
      m_manPanEnableTouchIndex = te->index;
   }
   else if (m_panButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_sceneActor->m_panButtonPressed = true;
         m_panTouchIndex = te->index;
         m_panStartPos = v;
      }
   }
   else if (m_reloadButtonRect.pointIn(p))
   {
      if (m_manualPan.m_manPanEnable)
      {
         m_mainActor->m_reloadPressed = true;
         m_reloadTouchIndex = te->index;
      }
   }
}


void ButtonDisplay::sceneUpHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;

   //logs::messageln("Up with index: %d", te->index);

   // Vector2 v = te->localPosition;
   // Point p = Point((int)v.x, (int)v.y);

   if (te->index == m_turnLeftTouchIndex)
   {
      m_sceneActor->m_turnLeftPressed = false;
      m_turnLeftTouchIndex = 0;
   }
   else if (te->index == m_turnRightTouchIndex)
   {
      m_sceneActor->m_turnRightPressed = false;
      m_turnRightTouchIndex = 0;
   }
   else if (te->index == m_boosterTouchIndex)
   {
      m_sceneActor->m_boosterPressed = false;
      m_boosterTouchIndex = 0;
   }
   else if (te->index == m_fireTouchIndex)
   {
      m_sceneActor->m_firePressed = false;
      m_fireTouchIndex = 0;
   }
   else if (te->index == m_pauseTouchIndex)
   {
      m_sceneActor->m_pausePressed = false;
      m_pauseTouchIndex = 0;
   }
   else if (te->index == m_zoomInTouchIndex)
   {
      m_sceneActor->m_zoomInPressed = false;
      m_zoomInTouchIndex = 0;
   }
   else if (te->index == m_zoomOutTouchIndex)
   {
      m_sceneActor->m_zoomOutPressed = false;
      m_zoomOutTouchIndex = 0;
   }
   else if (te->index == m_manPanEnableTouchIndex)
   {
      m_sceneActor->m_manPanEnablePressed = false;
      m_manPanEnableTouchIndex = 0;

      setManualPanButtonState();
   }
   else if (te->index == m_panTouchIndex)
   {
      m_sceneActor->m_panButtonPressed = false;
      m_sceneActor->m_panVector = Vector2(0.0f, 0.0f);
      m_panTouchIndex = 0;
   }
   else if (te->index == m_reloadTouchIndex)
   {
      m_mainActor->m_reloadPressed = false;
      m_reloadTouchIndex = 0;
   }
}

void ButtonDisplay::panMoveHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;

   if (te->index == m_panTouchIndex)
   {
      m_sceneActor->m_panVector = (te->localPosition - m_panStartPos) / 8.0f;
   }
}

void ButtonDisplay::sceneMoveHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;
   Vector2 v = te->localPosition;

   //logs::messageln("Move with index: %d", te->index);

}

void ButtonDisplay::setManualPanButtonState(void)
{
   if (m_manualPan.m_manPanEnable)
   {
      m_manPanEnableSprite->setAlpha(255);
      m_manPanSprite->setVisible(true);
      m_reloadSprite->setVisible(true);
      m_zoomInSprite->setVisible(true);
      m_zoomOutSprite->setVisible(true);
   }
   else
   {
      m_manPanEnableSprite->setAlpha(128);
      m_manPanSprite->setVisible(false);
      m_reloadSprite->setVisible(false);
      m_zoomInSprite->setVisible(false);
      m_zoomOutSprite->setVisible(false);
   }
}

void ButtonDisplay::createButtonOverlay(void)
{
   // Caluclate button geometrics
   m_turnLeftButtonRect = RectF(0.0f, g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_turnRightButtonRect = RectF(g_Layout.getButtonWidth(), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_boosterButtonRect = RectF(g_Layout.getXFromRight(0), g_Layout.getYFromBottom(1), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_fireButtonRect = RectF(g_Layout.getXFromRight(1), g_Layout.getYFromBottom(0), g_Layout.getButtonWidth(), g_Layout.getButtonWidth());

   m_pauseButtonRect = RectF(g_Layout.getXFromRight(0), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanEnableButtonRect = RectF(g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInButtonRect = RectF(g_Layout.getXFromRight(2) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutButtonRect = RectF(g_Layout.getXFromRight(4), 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_reloadButtonRect = RectF(g_Layout.getXFromRight(5) - g_Layout.getButtonWidth() / 2.0f, 0.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_panButtonRect = RectF(g_Layout.getViewPortBounds().x / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getViewPortBounds().y / 2.0f - g_Layout.getButtonWidth() / 2.0f, g_Layout.getButtonWidth(), g_Layout.getButtonWidth());

   // Define sprites
   spSprite turnLeftButton = new Sprite();
   turnLeftButton->setResAnim(m_hudResources->getResAnim("turn_left_button"));
   turnLeftButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnLeftButton->setPosition(m_turnLeftButtonRect.getLeftTop());
   turnLeftButton->setAnchor(0.0f, 0.0f);
   turnLeftButton->setTouchEnabled(false);
   turnLeftButton->attachTo(this);

   spSprite turnRightButton = new Sprite();
   turnRightButton->setResAnim(m_hudResources->getResAnim("turn_right_button"));
   turnRightButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   turnRightButton->setPosition(m_turnRightButtonRect.getLeftTop());
   turnRightButton->setAnchor(0.0f, 0.0f);
   turnRightButton->setTouchEnabled(false);
   turnRightButton->attachTo(this);

   spSprite fireButton = new Sprite();
   fireButton->setResAnim(m_hudResources->getResAnim("fire_button"));
   fireButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   fireButton->setPosition(m_fireButtonRect.getLeftTop());
   fireButton->setAnchor(0.0f, 0.0f);
   fireButton->setTouchEnabled(false);
   fireButton->attachTo(this);

   spSprite thursterButton = new Sprite();
   thursterButton->setResAnim(m_hudResources->getResAnim("booster_button"));
   thursterButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   thursterButton->setPosition(m_boosterButtonRect.getLeftTop());
   thursterButton->setAnchor(0.0f, 0.0f);
   thursterButton->setTouchEnabled(false);
   thursterButton->attachTo(this);

   spSprite pauseButton = new Sprite();
   pauseButton->setResAnim(m_hudResources->getResAnim("pause_button"));
   pauseButton->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   pauseButton->setPosition(m_pauseButtonRect.getLeftTop());
   pauseButton->setAnchor(0.0f, 0.0f);
   pauseButton->setTouchEnabled(false);
   pauseButton->attachTo(this);

   m_zoomOutSprite = new Sprite();
   m_zoomOutSprite->setResAnim(m_hudResources->getResAnim("zoom_out_button"));
   m_zoomOutSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomOutSprite->setPosition(m_zoomOutButtonRect.getLeftTop());
   m_zoomOutSprite->setAnchor(0.0f, 0.0f);
   m_zoomOutSprite->setTouchEnabled(false);
   m_zoomOutSprite->attachTo(this);

   m_zoomInSprite = new Sprite();
   m_zoomInSprite->setResAnim(m_hudResources->getResAnim("zoom_in_button"));
   m_zoomInSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_zoomInSprite->setPosition(m_zoomInButtonRect.getLeftTop());
   m_zoomInSprite->setAnchor(0.0f, 0.0f);
   m_zoomInSprite->setTouchEnabled(false);
   m_zoomInSprite->attachTo(this);

   m_manPanEnableSprite = new Sprite();
   m_manPanEnableSprite->setResAnim(m_hudResources->getResAnim("pan_button"));
   m_manPanEnableSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanEnableSprite->setPosition(m_manPanEnableButtonRect.getLeftTop());
   m_manPanEnableSprite->setAnchor(0.0f, 0.0f);
   m_manPanEnableSprite->setTouchEnabled(false);
   m_manPanEnableSprite->attachTo(this);

   m_manPanSprite = new Sprite();
   m_manPanSprite->setResAnim(m_hudResources->getResAnim("pan_button"));
   m_manPanSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_manPanSprite->setPosition(m_panButtonRect.getLeftTop());
   m_manPanSprite->setAnchor(0.0f, 0.0f);
   m_manPanSprite->setTouchEnabled(false);
   m_manPanSprite->attachTo(this);

   m_reloadSprite = new Sprite();
   m_reloadSprite->setResAnim(m_hudResources->getResAnim("reload_button"));
   m_reloadSprite->setSize(g_Layout.getButtonWidth(), g_Layout.getButtonWidth());
   m_reloadSprite->setPosition(m_reloadButtonRect.getLeftTop());
   m_reloadSprite->setAnchor(0.0f, 0.0f);
   m_reloadSprite->setTouchEnabled(false);
   m_reloadSprite->attachTo(this);

   setManualPanButtonState();
   m_sceneActor->setManualPan(&m_manualPan);

}


void ButtonDisplay::doUpdate(const UpdateState& us)
{
}
