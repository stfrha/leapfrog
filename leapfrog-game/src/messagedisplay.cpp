#include "messagedisplay.h"
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
   m_noPreviousMessages(true),
   m_fullHeight(false)
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


void MessageDisplay::cleanAndClearMessageDisplay(void)
{
   m_displayedMessages.clear();
   clearMessageDisplay();
}


void MessageDisplay::initialiseMessageDisplay(
   Resources* hudResources,
   Actor* mainActor,
   float highTop,
   float normalTop,
   float left,
   float right,
   float highBottom,
   float normalBottom,
   const float fontSize)
{
   m_highTop = highTop;
   m_normalTop = normalTop;
   m_left = left;
   m_right = right;
   m_highBottom = highBottom;
   m_normalBottom = normalBottom;

   m_hudResources = hudResources;

   m_fontSize = fontSize;

   float thickness = 1.0f;

   setResAnim(m_hudResources->getResAnim("display_thin"));
   setVerticalMode(Box9Sprite::STRETCHING);
   setHorizontalMode(Box9Sprite::STRETCHING);
   setGuides(8, 120, 8, 120);
   setTouchEnabled(false);
   setAnchor(0.0f, 0.0f);
   //setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   setPriority(250);
   attachTo(mainActor);

   spActor mdFrame = new Actor();
   mdFrame->setAnchor(0.0f, 0.0f);
   //mdFrame->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   mdFrame->setPosition(0.0f, 0.0f);
   mdFrame->attachTo(this);
   m_mdFrame = mdFrame.get();

   spClipRectActor messageActor = new ClipRectActor();
   messageActor->setAnchor(0.0f, 0.0f);
   //messageActor->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   messageActor->setPosition(0.0f, 0.0f);
   messageActor->attachTo(mdFrame);

   m_messageActor = messageActor.get();

   setFullHeight(m_fullHeight);
}

void MessageDisplay::setFullHeight(bool fullHeight)
{
   m_fullHeight = fullHeight;

   // Calculate the rect where the MessageDisplay is to be
   if (m_fullHeight)
   {
      m_messageDisplayHeight = m_highBottom - m_highTop;
      setPosition(m_left, m_highTop);
   }
   else
   {
      m_messageDisplayHeight = m_normalBottom - m_normalTop;
      setPosition(m_left, m_normalTop);
   }

   m_messageDisplayWidth = m_right - m_left;

   setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   m_messageActor->setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   m_mdFrame->setSize(m_messageDisplayWidth, m_messageDisplayHeight);

   redrawMessages();
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
   drawMessage(m_messageQueue[0]);
}


void MessageDisplay::drawMessage(const MessageItem& message)
{
   m_newBubble = new Box9Sprite();

   if (message.m_leftBubble)
   {
      m_newBubble->setResAnim(m_hudResources->getResAnim("msg_bbl_left"));
   }
   else
   {
      m_newBubble->setResAnim(m_hudResources->getResAnim("msg_bbl_right"));
   }


   m_newBubble->setVerticalMode(Box9Sprite::STRETCHING);
   m_newBubble->setHorizontalMode(Box9Sprite::STRETCHING);

   if (message.m_leftBubble)
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
   msgTextField->setText(message.m_message);
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

void MessageDisplay::instantTransit(void)
{
   // Now we know the size of the new entry. We can now move all other children 
   // up by that same amount

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
         actor->setPosition(pos.x, pos.y - m_newMessageHeight - 4.0f);
      }

      actor = actor->getNextSibling();
   }

   for (auto it = toOldToLiveList.begin(); it != toOldToLiveList.end(); ++it)
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
      m_displayedMessages.push_back(*(m_messageQueue.begin()));
      m_messageQueue.erase(m_messageQueue.begin());
   }
}

void MessageDisplay::redrawMessages(void)
{
   clearMessageDisplay();

   for (auto it = m_displayedMessages.begin(); it != m_displayedMessages.end(); ++it)
   {
      drawMessage(*it);

      if (m_noPreviousMessages)
      {
         m_noPreviousMessages = false;

         m_newBubble->setScale(1.0f);
         m_newBubble->attachTo(m_messageActor);
      }
      else
      {
         // New message but there are already old messages on display
         // we need to move old messages to make space for the new
         instantTransit();
         m_newBubble->setScale(1.0f);
         m_newBubble->attachTo(m_messageActor);
      }
   }
}

