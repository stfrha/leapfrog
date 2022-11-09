
#include "menuactor.h"
#include "sceneactor.h"
#include "gamestatus.h"
#include "layout.h"
#include "headdowndisplay.h"
#include "mainactor.h"
#include "graphicresources.h"

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
   setVerticalMode(Box9Sprite::STRETCHING);
   setHorizontalMode(Box9Sprite::STRETCHING);
   setGuides(8, 120, 8, 120);
   setSize(size);
   setPosition(pos);
   attachTo(menuParent);

   spTextField msgTextField = new TextField();

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

void MenuButton::doUpdate(const oxygine::UpdateState& us)
{

}


MenuActor::MenuActor(
   MainActor* mainActor) :
   m_mainActor(mainActor)
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

   spMenuButton b2 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 2 + buttonHeight * 1),
      Vector2(buttonWidth, buttonHeight),
      "Restart latest game");

   spMenuButton b3 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 3 + buttonHeight * 2),
      Vector2(buttonWidth, buttonHeight),
      "Start new game");

   spMenuButton b4 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 4 + buttonHeight * 3),
      Vector2(trioButtonWidth, buttonHeight),
      "Landing");

   spMenuButton b5 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1) + trioButtonWidth * 1 + sepTrioWidth * 1, sepHeight * 4 + buttonHeight * 3),
      Vector2(trioButtonWidth, buttonHeight),
      "Space");

   spMenuButton b6 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1) + trioButtonWidth * 2 + sepTrioWidth * 2, sepHeight * 4 + buttonHeight * 3),
      Vector2(trioButtonWidth, buttonHeight),
      "Orbit");

   spMenuButton b7 = new MenuButton(
      this,
      Vector2(g_Layout.getYFromTop(1), sepHeight * 6 + buttonHeight * 5),
      Vector2(buttonWidth, buttonHeight),
      "Exit");

   b1->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b1Clicked));
   b2->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b2Clicked));
   b3->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b3Clicked));
   b4->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b4Clicked));
   b5->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b5Clicked));
   b6->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b6Clicked));
   b7->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b7Clicked));

}

void MenuActor::doUpdate(const UpdateState &us)
{
}

void MenuActor::b1Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(1);
}

void MenuActor::b2Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(2);
}

void MenuActor::b3Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(3);
}

void MenuActor::b4Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(4);
}

void MenuActor::b5Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(5);
}

void MenuActor::b6Clicked(oxygine::Event* event)
{
   m_mainActor->buttonClicked(6);
}

void MenuActor::b7Clicked(oxygine::Event* event)
{
   //request quit
   m_mainActor->cleanUpAndQuit();
}
