
#include "menuactor.h"
#include "sceneactor.h"
#include "gamestatus.h"
#include "layout.h"
#include "headdowndisplay.h"
#include "mainactor.h"
#include "graphicresources.h"
#include "luainterface.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

using namespace oxygine;
using namespace std;


MenuButton::MenuButton(
   MenuActor* menuParent,
   const oxygine::Vector2& pos,
   const oxygine::Vector2& size,
   string caption /*  ,
   event eventToFireAtButtonCLick*/)
{
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   setVerticalMode(Box9Sprite::TILING_FULL);
   setHorizontalMode(Box9Sprite::TILING_FULL);
   setGuides(8, 120, 8, 120);
   setSize(size);
   setPosition(pos);
   attachTo(menuParent);

   m_text = new TextField();
   spTextField msgTextField = m_text;

   TextStyle style;
   style.multiline = false;
   style.vAlign = TextStyle::VerticalAlign::VALIGN_TOP;
   style.hAlign = TextStyle::HorizontalAlign::HALIGN_LEFT;
   style.font = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font");
   style.color = Color::White;
   style.fontSize = (int)(size.y / 2.0f);

   msgTextField->setStyle(style);
   msgTextField->setSize(Vector2(getSize().y - 40.0f - 8.0f, 0));
   msgTextField->setAnchor(0.0f, 0.0f);
   msgTextField->setPosition(8.0f, 8.0f);
   msgTextField->setText(caption);
   msgTextField->setColor(g_Layout.getPhosphorColor());

   const Rect& rect = msgTextField->getTextRect();

   msgTextField->attachTo(this);
}

void MenuButton::setCaption(const std::string& caption)
{
   m_text->setText(caption);
}

void MenuButton::doUpdate(const oxygine::UpdateState& us)
{

}


MenuActor::MenuActor()
{
   setPosition(0.0f, 0.0f);
   setAnchor(0.0f, 0.0f);
   setSize(g_HeadDownDisplay->getSize());
   attachTo(g_HeadDownDisplay);

   float buttonHeight = getSize().y / (6.0f + 7.0f / 5.0f);
   float sepHeight = buttonHeight / 5.0f;
   float buttonWidth = getSize().x - 2.0f * g_Layout.getButtonWidth();
   float trioButtonWidth = buttonWidth / (3.0f + 2.0f / 10.0f);
   float sepTrioWidth = trioButtonWidth / 10.0f;

   spMenuButton b1 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 1),
      Vector2(buttonWidth, buttonHeight),
      "Continue current game");
   b1->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b1Clicked));

   spMenuButton b2 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 2 + buttonHeight * 1),
      Vector2(buttonWidth, buttonHeight),
      "Restart latest game");
   b2->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b2Clicked));

   spMenuButton b3 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 3 + buttonHeight * 2),
      Vector2(buttonWidth, buttonHeight),
      "Start new game");
   b3->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b3Clicked));

   // The test row: < label >. The arrows step through the test starts,
   // the label starts the one shown (lua/test_starts.lua)
   float testRowY = sepHeight * 4 + buttonHeight * 3;
   float arrowWidth = buttonHeight * 1.2f;
   float testLabelWidth = buttonWidth - 2.0f * arrowWidth - 2.0f * sepTrioWidth;
   float testRowX = g_Layout.getYFromTop(1);

   spMenuButton prev = new MenuButton(
      this,
      Vector2(testRowX, testRowY),
      Vector2(arrowWidth, buttonHeight),
      "<");
   prev->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::testPrevClicked));

   m_testLabel = new MenuButton(
      this,
      Vector2(testRowX + arrowWidth + sepTrioWidth, testRowY),
      Vector2(testLabelWidth, buttonHeight),
      "");
   m_testLabel->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::testStartClicked));

   spMenuButton next = new MenuButton(
      this,
      Vector2(testRowX + arrowWidth + sepTrioWidth + testLabelWidth + sepTrioWidth, testRowY),
      Vector2(arrowWidth, buttonHeight),
      ">");
   next->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::testNextClicked));

   m_testCount = g_LuaInterface.lua_testStartCount();
   m_testIndex = g_MainActor->getTestStartIndex();
   if ((m_testIndex < 1) || (m_testIndex > m_testCount))
   {
      m_testIndex = 1;
   }
   showTestStart();

#if !(defined(__APPLE__) && TARGET_OS_IPHONE)
   // create/show the Exit menu item
   spMenuButton b7 = new MenuButton(
       this,
       Vector2(g_Layout.getYFromTop(1), sepHeight * 6 + buttonHeight * 5),
       Vector2(buttonWidth, buttonHeight),
       "Exit");

   b7->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b7Clicked));
#endif


}

void MenuActor::doUpdate(const UpdateState &us)
{
}

void MenuActor::b1Clicked(oxygine::Event* event)
{
   g_MainActor->buttonClicked(1);
}

void MenuActor::b2Clicked(oxygine::Event* event)
{
   g_MainActor->buttonClicked(2);
}

void MenuActor::b3Clicked(oxygine::Event* event)
{
   g_MainActor->buttonClicked(3);
}

void MenuActor::showTestStart(void)
{
   if (m_testCount <= 0)
   {
      m_testLabel->setCaption("No test starts");
      return;
   }

   char caption[160];
   snprintf(caption, sizeof(caption), "%d/%d  %s", m_testIndex, m_testCount,
      g_LuaInterface.lua_testStartLabel(m_testIndex).c_str());
   m_testLabel->setCaption(caption);
}

void MenuActor::testPrevClicked(oxygine::Event* event)
{
   if (m_testCount > 0)
   {
      m_testIndex = (m_testIndex <= 1) ? m_testCount : m_testIndex - 1;
      g_MainActor->setTestStartIndex(m_testIndex);
      showTestStart();
   }
}

void MenuActor::testNextClicked(oxygine::Event* event)
{
   if (m_testCount > 0)
   {
      m_testIndex = (m_testIndex >= m_testCount) ? 1 : m_testIndex + 1;
      g_MainActor->setTestStartIndex(m_testIndex);
      showTestStart();
   }
}

void MenuActor::testStartClicked(oxygine::Event* event)
{
   if (m_testCount > 0)
   {
      g_MainActor->testStartClicked(m_testIndex);
   }
}

void MenuActor::b7Clicked(oxygine::Event* event)
{
   //request quit
   g_MainActor->cleanUpAndQuit();
}
