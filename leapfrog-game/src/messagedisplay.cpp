#include "messagedisplay.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spMessageDisplay g_MessageDisplay;


MessageDisplay::MessageDisplay() 
{
}

void MessageDisplay::clearMessageDisplay(void)
{
}


void MessageDisplay::initialiseMessageDisplay(
   Resources* gameResources,
   SceneActor* sceneActor,
   const Vector2& topLeft, 
   const Vector2& bottomRight)
{
   m_gameResources = gameResources;

   // Calculate the rect where the MessageDisplay is to be
   m_messageDisplayWidth = bottomRight.x - topLeft.x;
   m_messageDisplayHeight = bottomRight.y - topLeft.y;

   float thickness = 1.0f;

   setAnchor(0.0f, 0.0f);
   setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   setPosition(topLeft);
   attachTo(sceneActor->getParent());

   spActor mdFrame = new Actor();
   mdFrame->setAnchor(0.0f, 0.0f);
   mdFrame->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   mdFrame->setPosition(0.0f, 0.0f);
   mdFrame->attachTo(this);

   m_messageActor = new ClipRectActor();
   m_messageActor->setAnchor(0.0f, 0.0f);
   m_messageActor->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   m_messageActor->setPosition(0.0f, 0.0f);
   m_messageActor->attachTo(mdFrame);

   spColorRectSprite theBar = new ColorRectSprite();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   theBar->setPosition(0.0f, 0.0f);
   theBar->setColor(Color::Fuchsia);
   theBar->setAlpha(32);
   theBar->attachTo(mdFrame);

   spColorRectSprite top = new ColorRectSprite();
   top->setColor(Color::Fuchsia);
   top->setAnchor(0.0f, 0.0f);
   top->setSize(m_messageDisplayWidth, thickness);
   top->setPosition(0.0f, 0.0f);
   top->attachTo(mdFrame);

   spColorRectSprite bottom = new ColorRectSprite();
   bottom->setColor(Color::Fuchsia);
   bottom->setAnchor(0.0f, 0.0f);
   bottom->setSize(m_messageDisplayWidth, thickness);
   bottom->setPosition(0.0f, m_messageDisplayHeight - thickness);
   bottom->attachTo(mdFrame);

   spColorRectSprite left = new ColorRectSprite();
   left->setColor(Color::Fuchsia);
   left->setAnchor(0.0f, 0.0f);
   left->setSize(thickness, m_messageDisplayHeight);
   left->setPosition(0.0f, 0.0f);
   left->attachTo(mdFrame);

   spColorRectSprite right = new ColorRectSprite();
   right->setColor(Color::Fuchsia);
   right->setAnchor(0.0f, 0.0f);
   right->setSize(thickness, m_messageDisplayHeight);
   right->setPosition(m_messageDisplayWidth - thickness, 0.0f);
   right->attachTo(mdFrame);

}

void MessageDisplay::doUpdate(const oxygine::UpdateState& us)
{
}

void MessageDisplay::initMessage(
   bool leftBubble,
   std::string& messageString,
   std::string& senderString)
{
   spBox9Sprite bubble = new Box9Sprite();

   if (leftBubble)
   {
      bubble->setResAnim(m_gameResources->getResAnim("msg_bbl_left"));
   }
   else
   {
      bubble->setResAnim(m_gameResources->getResAnim("msg_bbl_right"));
   }


   bubble->setVerticalMode(Box9Sprite::STRETCHING);
   bubble->setHorizontalMode(Box9Sprite::STRETCHING);

   if (leftBubble)
   {
      bubble->setAnchor(0.0f, 0.0f);
      bubble->setPosition(4.0f, 4.0f);
   }
   else
   {
      bubble->setAnchor(1.0f, 0.0f);
      bubble->setPosition(m_messageDisplayWidth - 4.0f, 4.0f);
   }

   spTextField msgTextField = new TextField();

   TextStyle style;
   style.multiline = true;
   style.vAlign = TextStyle::VerticalAlign::VALIGN_TOP;
   style.hAlign = TextStyle::HorizontalAlign::HALIGN_LEFT;
   style.font = m_gameResources->getResFont("lf_font");
   style.color = Color::White;
   style.fontSize = 14.0f;

   msgTextField->setStyle(style);
   msgTextField->setSize(Vector2(m_messageDisplayWidth - 40.0f - 8.0f, 0));
   msgTextField->setAnchor(0.0f, 0.0f);
   msgTextField->setPosition(8.0f, 8.0f);
   msgTextField->setText(messageString);

   const Rect& rect = msgTextField->getTextRect();
  
   float newMessageHeight = rect.size.y + 20.0f;
   
   bubble->setSize(rect.size.x + 20.0f, newMessageHeight);

   // Now we now the size of the new entry. We can now move all other children 
   // down by that same amount

   spActor actor = m_messageActor->getFirstChild();

   while (actor)
   {
      Vector2 pos = actor->getPosition();

      // Check if message is no longer visible
      if (pos.y + newMessageHeight > m_messageDisplayHeight)
      {
         // TODO: remove actor
      }

      actor->setPosition(pos.x, pos.y + newMessageHeight + 4.0f);
      actor = actor->getNextSibling();
   }
   
   // When all old messages are pushed down, 
   // we add the new actors
   bubble->attachTo(m_messageActor);
   msgTextField->attachTo(bubble);

}
