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
   bool leftBubble,
   int preWait,
   int postWait) :
   m_message(message),
   m_sender(sender),
   m_leftBubble(leftBubble),
   m_preWait(preWait),
   m_postWait(postWait)
{

}


MessageDisplay::MessageDisplay()  :
   m_state(idle),
   m_noPreviousMessages(true)
{
}

void MessageDisplay::clearMessageDisplay(void)
{
   m_noPreviousMessages = true;
   m_messageQueue.clear();

   // Clear the m_messageActor from bubble actors
   spActor actor = m_messageActor->getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
         actor->detach();
      }
      actor = next;
   }
}


void MessageDisplay::initialiseMessageDisplay(
   Resources* hudResources,
   Actor* mainActor,
   const Vector2& topLeft, 
   const Vector2& bottomRight,
   const float fontSize)
{
   m_hudResources = hudResources;

   // Calculate the rect where the MessageDisplay is to be
   m_messageDisplayWidth = bottomRight.x - topLeft.x;
   m_messageDisplayHeight = bottomRight.y - topLeft.y;
   m_fontSize = fontSize;

   float thickness = 1.0f;

   setAnchor(0.0f, 0.0f);
   setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   setPosition(topLeft);
   setPriority(250);
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

   spBox9Sprite hej = new Box9Sprite();
   hej->setResAnim(m_hudResources->getResAnim("display_thin"));
   hej->setVerticalMode(Box9Sprite::STRETCHING);
   hej->setHorizontalMode(Box9Sprite::STRETCHING);
   hej->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   hej->setAnchor(0.0f, 0.0f);
   hej->setPosition(0.0f, 0.0f);
   hej->setGuides(8, 120, 8, 120);
   hej->setTouchEnabled(false);
   hej->attachTo(mdFrame);

}

void MessageDisplay::doUpdate(const oxygine::UpdateState& us)
{
   // We only handle idle state here. The other states
   // are active during tween animation and the end of 
   // tween handlers will set state back to idel (eventually).

   switch (m_state)
   {
   case idle:
      if (m_messageQueue.size() > 0)
      {
         if ((m_messageQueue[0].m_preWait > 0))
         {
            m_ticks = 0;
            m_state = preWait;
         }
         else
         {
            prepareFirstInQueue();
         }
      }
      break;

   case preWait:
      m_ticks++;
      if (m_ticks > m_messageQueue[0].m_preWait)
      {
         prepareFirstInQueue();
      }
      break;
   case postWait:
      m_ticks++;
      if (m_ticks > m_messageQueue[0].m_postWait)
      {
         m_state = idle;
         m_messageQueue.erase(m_messageQueue.begin());
      }
      break;
   }
}

void MessageDisplay::initMessage(
   bool leftBubble,
   const char* messageString,
   const char* senderString,
   int preWait,
   int postWait)
{
   MessageItem newMessage(messageString, senderString, leftBubble, preWait, postWait);

   m_messageQueue.push_back(newMessage);
}

void MessageDisplay::prepareFirstInQueue(void)
{
   processFirstInQueue();

   if (m_noPreviousMessages)
   {
      m_noPreviousMessages = false;

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

void MessageDisplay::processFirstInQueue(void)
{
   m_newBubble = new Box9Sprite();

   if (m_messageQueue[0].m_leftBubble)
   {
      m_newBubble->setResAnim(m_hudResources->getResAnim("msg_bbl_left"));
   }
   else
   {
      m_newBubble->setResAnim(m_hudResources->getResAnim("msg_bbl_right"));
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
   style.font = m_hudResources->getResFont("lf_font");
   style.color = Color::White;
   style.fontSize = (int)m_fontSize;

   msgTextField->setStyle(style);
   msgTextField->setSize(Vector2(m_messageDisplayWidth - 40.0f - 8.0f, 0));
   msgTextField->setAnchor(0.0f, 0.0f);
   msgTextField->setPosition(8.0f, 8.0f);
   msgTextField->setText(m_messageQueue[0].m_message);
   msgTextField->setColor(g_Layout.getPhosphorColor());


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

   vector<spActor> toOldToLiveList;

   while (actor)
   {
      Vector2 pos = actor->getPosition();

      // Check if message is no longer visible
      if (pos.y < 0)
      {
         // TODO: remove actor
         toOldToLiveList.push_back(actor);
      }
      else
      {
         tween = actor->addTween(Actor::TweenPosition(pos.x, pos.y - m_newMessageHeight - 4.0f), 500);
      }

      actor = actor->getNextSibling();
   }

   if (tween)
   {
      // Lets use the last tween to get a call back when it is finished
      tween->setDoneCallback(CLOSURE(this, &MessageDisplay::atTransitFinished));
   }

   for (auto it = toOldToLiveList.begin(); it !=  toOldToLiveList.end(); ++it)
   {
      (*it)->detach();
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
   if (m_messageQueue[0].m_postWait > 0)
   {
      m_ticks = 0;
      m_state = postWait;
   }
   else
   {
      m_state = idle;
      m_messageQueue.erase(m_messageQueue.begin());
   }

}
