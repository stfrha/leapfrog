
#include "menuactor.h"
#include "sceneactor.h"
#include "gamestatus.h"
#include "layout.h"
#include "headdowndisplay.h"
#include "mainactor.h"

using namespace oxygine;
using namespace std;


MenuButton::MenuButton(
   oxygine::Resources& hudResources,
   MenuActor* menuParent,
   const oxygine::Vector2& pos,
   const oxygine::Vector2& size,
   string caption /*  ,
   event eventToFireAtButtonCLick*/)
{
   setResAnim(hudResources.getResAnim("display_fat"));
   setVerticalMode(Box9Sprite::STRETCHING);
   setHorizontalMode(Box9Sprite::STRETCHING);
   setSize(size);
   setPosition(pos);
   attachTo(menuParent);

   spTextField msgTextField = new TextField();

   TextStyle style;
   style.multiline = true;
   style.vAlign = TextStyle::VerticalAlign::VALIGN_TOP;
   style.hAlign = TextStyle::HorizontalAlign::HALIGN_LEFT;
   style.font = hudResources.getResFont("lf_font");
   style.color = Color::White;
   style.fontSize = g_Layout.getDefaultFontSize();

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
   Resources& hudResources,
   MainActor* parent,
   const Vector2& pos,
   const Vector2& size) :
   m_mainActor(parent)
{
   setPosition(pos);
   setSize(size);
   attachTo(g_HeadDownDisplay);

   spMenuButton b1 = new MenuButton(
      hudResources,
      this,
      Vector2(10.0f, 10.0f), Vector2(100.0f, 100.0f),
      "Press me, please!!");

   b1->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MenuActor::b1Clicked));

}

void MenuActor::doUpdate(const UpdateState &us)
{
}

void MenuActor::b1Clicked(oxygine::Event* event)
{
   m_mainActor->buttonB1Clicked();
}
