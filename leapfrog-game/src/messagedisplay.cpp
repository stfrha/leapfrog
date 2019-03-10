#include "messagedisplay.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spMessageDisplay g_MessageDisplay;



MessageItem::MessageItem(
   string message,
   string sender,
   bool leftBubble) :
   m_message(message),
   m_sender(sender),
   m_leftBubble(leftBubble)
{

}


MessageDisplay::MessageDisplay()  :
   m_state(idle),
   m_noMessagesYet(true)
{
}

void MessageDisplay::clearMessageDisplay(void)
{
}


void MessageDisplay::initialiseMessageDisplay(
   Resources* gameResources,
   Actor* mainActor,
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
   attachTo(mainActor);

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
   // We only handle idle state here. The other states
   // are active during tween animation and the end of 
   // tween handlers will set state back to idel (eventually).
   if (m_state == idle)
   {
      if (m_messageQueue.size() > 0)
      {
         processFirstInQueue();

         if (m_noMessagesYet)
         {
            m_noMessagesYet = false;

            m_state = newBubbleAnimation;

            startMessageAnimation();
         }
         else
         {
            // New message but there are already old messages on display
            // we need to move old messages to make space for the new
            startTransit();

            m_state = inTransit;
         }
      }
   }
}

void MessageDisplay::initMessage(
   bool leftBubble,
   const char* messageString,
   const char* senderString)
{
   MessageItem newMessage(messageString, senderString, leftBubble);

   m_messageQueue.push_back(newMessage);
}

void MessageDisplay::processFirstInQueue(void)
{
   m_newBubble = new Box9Sprite();

   if (m_messageQueue[0].m_leftBubble)
   {
      m_newBubble->setResAnim(m_gameResources->getResAnim("msg_bbl_left"));
   }
   else
   {
      m_newBubble->setResAnim(m_gameResources->getResAnim("msg_bbl_right"));
   }


   m_newBubble->setVerticalMode(Box9Sprite::STRETCHING);
   m_newBubble->setHorizontalMode(Box9Sprite::STRETCHING);

   if (m_messageQueue[0].m_leftBubble)
   {
      m_newBubble->setAnchor(0.0f, 1.0f);
      m_newBubble->setPosition(4.0f, m_messageDisplayHeight - 4.0f);
   }
   else
   {
      m_newBubble->setAnchor(1.0f, 1.0f);
      m_newBubble->setPosition(m_messageDisplayWidth - 4.0f, m_messageDisplayHeight - 4.0f);
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
   msgTextField->setText(m_messageQueue[0].m_message);

   const Rect& rect = msgTextField->getTextRect();

   m_newMessageHeight = rect.size.y + 20.0f;

   m_newBubble->setSize(rect.size.x + 20.0f, m_newMessageHeight);

   msgTextField->attachTo(m_newBubble);

}

void MessageDisplay::startTransit(void)
{
   // Now we know the size of the new entry. We can now move all other children 
   // up by that same amount

   spTween tween;

   spActor actor = m_messageActor->getFirstChild();

   while (actor)
   {
      Vector2 pos = actor->getPosition();

      // Check if message is no longer visible
      if (pos.y + m_newMessageHeight > m_messageDisplayHeight)
      {
         // TODO: remove actor
         //actor->detach();
      }

//      actor->setPosition(pos.x, pos.y + m_newMessageHeight + 4.0f);

      tween = actor->addTween(Actor::TweenPosition(pos.x, pos.y - m_newMessageHeight - 4.0f), 500);

      actor = actor->getNextSibling();
   }

   if (tween)
   {
      // Lets use the last tween to get a call back when it is finished
      tween->setDoneCallback(CLOSURE(this, &MessageDisplay::atTransitFinished));
   }
}

void MessageDisplay::startMessageAnimation(void)
{
   // When all old messages are pushed up, 
   // we add the new actors
   m_newBubble->setScale(0.0f);
   m_newBubble->attachTo(m_messageActor);

   spTween tween = m_newBubble->addTween(Actor::TweenScale(1.0f, 1.0f), 250, 1, false, 0, Tween::EASE::ease_inOutBack);

   tween->setDoneCallback(CLOSURE(this, &MessageDisplay::atNewBubbleFinished));
}


void MessageDisplay::atTransitFinished(oxygine::Event* event)
{
   m_state = newBubbleAnimation;

   startMessageAnimation();
}

void MessageDisplay::atNewBubbleFinished(oxygine::Event* event)
{
   m_messageQueue.erase(m_messageQueue.begin());
   m_state = idle;
}
