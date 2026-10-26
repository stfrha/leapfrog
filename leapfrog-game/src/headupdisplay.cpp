#include "headupdisplay.h"

#include "mainactor.h"
#include "sceneactor.h"
#include "statusbar.h"
#include "layout.h"

using namespace std;
using namespace oxygine;

// Instanciate global instance of the HUD
spHeadUpDisplay g_headUpDisplay;

HeadUpDisplay::HeadUpDisplay() :
   m_sceneActor(NULL),
   m_buttonDisplay(NULL),
   m_statusBarActor(NULL),
   m_lastCheatTap(0),
   m_lastCheatTapTime(0)
{
}

bool HeadUpDisplay::checkCheatTap(const Vector2& pos)
{
   // A double tap (two taps within 0.4 s) on the fuel bar fills the tank,
   // on the credits gives 100 credits. Only when the status bars show.
   if ((m_statusBarActor == NULL) || !m_statusBarActor->getVisible() || (m_sceneActor == NULL))
   {
      return false;
   }

   int tap = 0;

   if (m_fuelCheatRect.pointIn(pos))
   {
      tap = 1;
   }
   else if (m_creditCheatRect.pointIn(pos))
   {
      tap = 2;
   }

   if (tap == 0)
   {
      m_lastCheatTap = 0;
      return false;
   }

   timeMS now = getTimeMS();

   if ((tap == m_lastCheatTap) && (now - m_lastCheatTapTime <= 400))
   {
      if (tap == 1)
      {
         m_sceneActor->cheatFuel();
      }
      else
      {
         m_sceneActor->cheatCredits();
      }

      // A third tap starts a new double tap
      m_lastCheatTap = 0;
   }
   else
   {
      m_lastCheatTap = tap;
      m_lastCheatTapTime = now;
   }

   return true;
}

void HeadUpDisplay::initialiseHeadUpDisplay(
   SceneActor* sceneActor,
   const oxygine::Vector2& topLeft,
   const oxygine::Vector2& bottomRight)
{
   m_sceneActor = sceneActor;

   spButtonDisplay buttonDisplay = new ButtonDisplay();

   buttonDisplay->initialiseButtonDisplay(
      m_sceneActor,
      topLeft,
      bottomRight);

   buttonDisplay->attachTo(this);

   m_buttonDisplay = buttonDisplay.get();

   spActor statusBarActor = new Actor();

   statusBarActor->setAnchor(0.0f, 0.0f);
   statusBarActor->setPosition(0.0f, 0.0f);
   statusBarActor->setSize(getStage()->getSize());
   statusBarActor->attachTo(this);

   m_statusBarActor = statusBarActor.get();

   setAnchor(0.0f, 0.0f);
   setPosition(0.0f, 0.0f);
   setSize(getStage()->getSize());

   attachTo(g_MainActor);

   // MainActor did this for every start of scene, is this necessary?
   addTouchDownListener(CLOSURE(this, &HeadUpDisplay::sceneDownHandler));
   addTouchUpListener(CLOSURE(this, &HeadUpDisplay::sceneUpHandler));
   addEventListener(TouchEvent::MOVE, (CLOSURE(this, &HeadUpDisplay::panMoveHandler)));

}

void HeadUpDisplay::cleanAndRemove(void)
{
   removeChildren();
   //m_buttonDisplay->detach();
   detach();
}

void HeadUpDisplay::showStatusBars(bool show)
{
   m_statusBarActor->setVisible(show);
}

void HeadUpDisplay::showButtons(bool show)
{
   m_buttonDisplay->setVisible(show);
}

void HeadUpDisplay::registerLeapfrog(LeapFrog* leapfrog, bool hideShield)
{
   if (leapfrog == NULL)
   {
      return;
   }

   float initShots = 100.0f;
   float initFuel = 100.0f;
   float initShield = 100.0f;
   float initDamage = 100.0f;
   float initCredits = 0.0f;

   if (leapfrog != NULL)
   {
      initShots = (float)g_GameStatus.getResources()->getShots();
      initFuel = g_GameStatus.getResources()->getFuel();
      initShield = g_GameStatus.getResources()->getShield();
      initDamage = g_GameStatus.getResources()->getDamage();
      initCredits = (float)g_GameStatus.getResources()->getCredits();
   }

   spStatusBar shotsBar = new StatusBar(
      leapfrog,
      m_sceneActor,
      m_statusBarActor,
      5,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1)),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initShots,
      "Ammo:",
      ObjectResourcesTypeEnum::shots);

   // Where the fuel bar and the credits are, for the cheat taps
   m_fuelCheatRect = RectF(
      g_Layout.getXFromRight(1),
      g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f + 2.0f,
      g_Layout.getButtonWidth() * 2.0f,
      g_Layout.getButtonWidth() / 2.0f);

   m_creditCheatRect = RectF(
      g_Layout.getXFromRight(1),
      g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 4.0f + 2.0f,
      g_Layout.getButtonWidth() * 2.0f,
      g_Layout.getButtonWidth() / 2.0f);

   spStatusBar fuelBar = new StatusBar(
      leapfrog,
      m_sceneActor,
      m_statusBarActor,
      7,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initFuel,
      "Fuel:",
      ObjectResourcesTypeEnum::fuel);

   spStatusBar shieldBar = new StatusBar(
      leapfrog,
      m_sceneActor,
      m_statusBarActor,
      6,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 2.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initShield,
      "Shield:",
      ObjectResourcesTypeEnum::shield);

   spStatusBar damageBar = new StatusBar(
      leapfrog,
      m_sceneActor,
      m_statusBarActor,
      9,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 3.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      100.0f,
      initDamage,
      "Damage:",
      ObjectResourcesTypeEnum::damage);

   spStatusLiteral creditBar = new StatusLiteral(
      leapfrog,
      m_sceneActor,
      m_statusBarActor,
      8,
      Vector2(g_Layout.getXFromRight(1), g_Layout.getYFromTop(1) + g_Layout.getButtonWidth() / 2.0f * 4.0f + 2.0f),
      Vector2(g_Layout.getButtonWidth() * 2.0f, g_Layout.getButtonWidth() / 2.0f),
      (int)g_Layout.getDefaultFontSize(),
      (int)initCredits,
      "Credits:",
      ObjectResourcesTypeEnum::credits);

   if (hideShield)
   {
      shieldBar->setAlpha(128);
   }
}

void HeadUpDisplay::doUpdate(const UpdateState& us)
{
}

void HeadUpDisplay::sceneDownHandler(Event* event)
{
   TouchEvent* te = (TouchEvent*)event;

   if (checkCheatTap(te->localPosition))
   {
      return;
   }

   m_buttonDisplay->sceneDownHandler(event);
}


void HeadUpDisplay::sceneUpHandler(Event* event)
{
   m_buttonDisplay->sceneUpHandler(event);
}

void HeadUpDisplay::panMoveHandler(Event* event)
{
   m_buttonDisplay->panMoveHandler(event);
}

void HeadUpDisplay::sceneMoveHandler(Event* event)
{
   m_buttonDisplay->sceneMoveHandler(event);
}
