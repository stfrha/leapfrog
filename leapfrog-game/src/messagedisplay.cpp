#include "messagedisplay.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"
#include "graphicresources.h"
#include "mainactor.h"

#include <map>

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spMessageDisplay g_MessageDisplay;

namespace
{
   // Messages kept for scrolling back in the expanded display
   const size_t c_maxKeptMessages = 40;

   // Expanded display: zoom, and width as a part of the screen width
   const float c_expandedZoom = 1.1f;
   const float c_expandedWidthPart = 0.6f;

   // The bubbles are clipped this far inside the top and bottom of the
   // display, so they don't draw over the frame line (display_thin)
   const float c_clipInset = 4.0f;

   // Testing: true puts the message text in the affiliation colour too,
   // false keeps it phosphor green (only the bubble frame is coloured).
   // Uncategorised senders (none) are phosphor green either way.
   const bool c_textInAffiliationColour = true;

   std::map<std::string, Affiliation>& senderAffiliations(void)
   {
      static std::map<std::string, Affiliation> s_senderAffiliations;
      return s_senderAffiliations;
   }
}

void MessageDisplay::setSenderAffiliation(const std::string& sender, Affiliation affiliation)
{
   senderAffiliations()[sender] = affiliation;
}

Affiliation MessageDisplay::getSenderAffiliation(const std::string& sender)
{
   auto it = senderAffiliations().find(sender);
   if (it == senderAffiliations().end())
   {
      return Affiliation::none;
   }
   return it->second;
}

MessageItem::MessageItem(
   string message,
   string sender,
   bool leftBubble,
   Affiliation affiliation,
   int preWait,
   int postWait) :
   m_message(message),
   m_sender(sender),
   m_leftBubble(leftBubble),
   m_affiliation(affiliation),
   m_preWait(preWait),
   m_postWait(postWait)
{
}


MessageDisplay::MessageDisplay()  :
   m_scrollActor(NULL),
   m_scrollOffset(0.0f),
   m_dragging(false),
   m_dragLastY(0.0f),
   m_state(idle),
   m_noPreviousMessages(true),
   m_fullHeight(false),
   m_expanded(false)
{
}

void MessageDisplay::clearMessageDisplay(void)
{
   m_noPreviousMessages = true;
   m_messageQueue.clear();

   // Clear the scroll actor from bubble actors
   setScrollOffset(0.0f);
   spActor actor = m_scrollActor->getFirstChild();

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
   if (m_expanded)
   {
      collapse(false);
   }

   m_displayedMessages.clear();
   clearMessageDisplay();
}


void MessageDisplay::initialiseMessageDisplay(
   Actor* parentActor,
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

   m_fontSize = fontSize;

   float thickness = 1.0f;

   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   setVerticalMode(Box9Sprite::TILING_FULL);
   setHorizontalMode(Box9Sprite::TILING_FULL);

   // Touching the display enlarges it (see expand). It is above the
   // HUD, so these touches don't reach the game controls.
   setTouchEnabled(true);
   setTouchChildrenEnabled(false);
   addTouchDownListener(CLOSURE(this, &MessageDisplay::touchDownHandler));
   addEventListener(TouchEvent::MOVE, CLOSURE(this, &MessageDisplay::touchMoveHandler));
   addTouchUpListener(CLOSURE(this, &MessageDisplay::touchUpHandler));
   addEventListener(TouchEvent::WHEEL_DIR, CLOSURE(this, &MessageDisplay::wheelHandler));
   setAnchor(0.0f, 0.0f);
   //setSize(m_messageDisplayWidth, m_messageDisplayHeight);
   setPriority(250);
   attachTo(parentActor);

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

   spActor scrollActor = new Actor();
   scrollActor->setAnchor(0.0f, 0.0f);
   scrollActor->setPosition(0.0f, -c_clipInset);
   scrollActor->attachTo(messageActor);
   m_scrollActor = scrollActor.get();

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

   applySize(m_right - m_left, m_messageDisplayHeight);
}

void MessageDisplay::applySize(float width, float height)
{
   m_messageDisplayWidth = width;
   m_messageDisplayHeight = height;

   setSize(m_messageDisplayWidth, m_messageDisplayHeight);

   // The clip rect is inset at the top and bottom; the scroll actor is
   // moved back up by the same amount, so the bubbles keep their
   // coordinates in the display (see setScrollOffset)
   m_messageActor->setPosition(0.0f, c_clipInset);
   m_messageActor->setSize(m_messageDisplayWidth, m_messageDisplayHeight - 2.0f * c_clipInset);
   m_mdFrame->setSize(m_messageDisplayWidth, m_messageDisplayHeight);

   if (m_tapLabel)
   {
      m_tapLabel->setPosition(m_messageDisplayWidth / 2.0f, 0.0f);
   }

   redrawMessages();
}

void MessageDisplay::touchDownHandler(oxygine::Event* event)
{
   if (m_expanded)
   {
      // Start dragging to scroll
      TouchEvent* te = static_cast<TouchEvent*>(event);
      m_dragging = true;
      m_dragLastY = te->localPosition.y;
   }
   else if (g_MainActor && g_MainActor->canPauseForMessages())
   {
      expand();
   }
}

void MessageDisplay::touchMoveHandler(oxygine::Event* event)
{
   if (!m_expanded || !m_dragging)
   {
      return;
   }

   // localPosition is in the display's own (unscaled) coordinates, like
   // the bubbles, so the content follows the finger
   TouchEvent* te = static_cast<TouchEvent*>(event);
   float y = te->localPosition.y;
   setScrollOffset(m_scrollOffset + (y - m_dragLastY));
   m_dragLastY = y;
}

void MessageDisplay::touchUpHandler(oxygine::Event* event)
{
   m_dragging = false;
}

void MessageDisplay::wheelHandler(oxygine::Event* event)
{
   // Mouse wheel on the desktop builds
   if (m_expanded)
   {
      TouchEvent* te = static_cast<TouchEvent*>(event);
      setScrollOffset(m_scrollOffset + te->wheelDirection.y * m_fontSize * 2.0f);
   }
}

void MessageDisplay::dimmerTouchHandler(oxygine::Event* event)
{
   if (m_expanded)
   {
      collapse(true);
   }
}

// 0 shows the newest message at the bottom; larger values move the
// bubbles down to show older ones, up to the oldest kept bubble at the top
void MessageDisplay::setScrollOffset(float offset)
{
   float maxOffset = getMaxScrollOffset();

   if (offset > maxOffset)
   {
      offset = maxOffset;
   }
   if (offset < 0.0f)
   {
      offset = 0.0f;
   }

   m_scrollOffset = offset;

   if (m_scrollActor)
   {
      m_scrollActor->setY(m_scrollOffset - c_clipInset);
   }
}

float MessageDisplay::getMaxScrollOffset(void)
{
   if (m_scrollActor == NULL)
   {
      return 0.0f;
   }

   // Bubbles are anchored at their bottom edge. Nothing to scroll while
   // the oldest bubble is still within the top gap (inside the clip).
   const float topGap = c_clipInset + 4.0f;
   float top = topGap;
   spActor actor = m_scrollActor->getFirstChild();
   while (actor)
   {
      float bubbleTop = actor->getY() - actor->getHeight() * actor->getScaleY();
      if (bubbleTop < top)
      {
         top = bubbleTop;
      }
      actor = actor->getNextSibling();
   }

   // Leave the same small gap above the oldest bubble as below the newest
   float maxOffset = topGap - top;
   return maxOffset > 0.0f ? maxOffset : 0.0f;
}

// Keeps the last c_maxKeptMessages bubbles. (m_displayedMessages is
// trimmed in atNewBubbleFinished, not here: a redraw loops over it.)
void MessageDisplay::pruneOldBubbles(void)
{
   size_t count = 0;
   for (spActor a = m_scrollActor->getFirstChild(); a; a = a->getNextSibling())
   {
      count++;
   }

   while (count > c_maxKeptMessages)
   {
      // Oldest first
      m_scrollActor->getFirstChild()->detach();
      count--;
   }
}

void MessageDisplay::expand(void)
{
   const float zoom = c_expandedZoom;
   Vector2 viewport = g_Layout.getViewPortBounds();
   float margin = g_Layout.getButtonWidth() * 0.2f;

   m_expanded = true;
   g_MainActor->setMessagePause(true);

   // Dim the paused game. Touching it (or the display) continues.
   m_dimmer = new ColorRectSprite();
   m_dimmer->setColor(Color(0, 0, 0, 255));
   m_dimmer->setAlpha(0);
   m_dimmer->setAnchor(0.0f, 0.0f);
   m_dimmer->setPosition(0.0f, 0.0f);
   m_dimmer->setSize(viewport);
   m_dimmer->setPriority(getPriority() - 1);
   m_dimmer->addTouchDownListener(CLOSURE(this, &MessageDisplay::dimmerTouchHandler));
   m_dimmer->attachTo(getParent());
   m_dimmer->addTween(Actor::TweenAlpha(140), 250);

   // Wider than normal and all the height there is, so more text fits,
   // slightly scaled up and centred. The bubbles are redrawn for the new
   // width (applySize).
   float height = (viewport.y - 2.0f * margin) / zoom;
   float width = viewport.x * c_expandedWidthPart / zoom;
   if (width < m_right - m_left)
   {
      width = m_right - m_left;
   }

   removeTweens();
   applySize(width, height);

   createTapLabel();

   Vector2 target((viewport.x - width * zoom) / 2.0f, margin);
   addTween(Actor::TweenPosition(target), 250, 1, false, 0, Tween::EASE::ease_outQuad);
   addTween(Actor::TweenScale(zoom), 250, 1, false, 0, Tween::EASE::ease_outQuad);
}

void MessageDisplay::collapse(bool animate)
{
   m_expanded = false;
   m_dragging = false;

   if (m_tapLabel)
   {
      m_tapLabel->detach();
      m_tapLabel = NULL;
   }

   if (m_dimmer)
   {
      m_dimmer->detach();
      m_dimmer = NULL;
   }

   removeTweens();

   if (animate)
   {
      Vector2 target(m_left, m_fullHeight ? m_highTop : m_normalTop);
      addTween(Actor::TweenPosition(target), 200, 1, false, 0, Tween::EASE::ease_inQuad);
      spTween tween = addTween(Actor::TweenScale(1.0f), 200, 1, false, 0, Tween::EASE::ease_inQuad);
      tween->setDoneCallback(CLOSURE(this, &MessageDisplay::collapseTweenDone));
   }
   else
   {
      setScale(1.0f);
      setFullHeight(m_fullHeight);
   }

   if (g_MainActor)
   {
      g_MainActor->setMessagePause(false);
   }
}

void MessageDisplay::collapseTweenDone(oxygine::Event* event)
{
   setScale(1.0f);
   setFullHeight(m_fullHeight);
}

void MessageDisplay::createTapLabel(void)
{
   // Small text centred on the top edge. A dark plate behind it hides
   // the frame line where the text sits.
   int fontSize = (int)(m_fontSize * 0.7f);
   if (fontSize < 8)
   {
      fontSize = 8;
   }

   m_tapLabel = new Actor();
   m_tapLabel->setPosition(m_messageDisplayWidth / 2.0f, 0.0f);
   m_tapLabel->setTouchEnabled(false, false);
   m_tapLabel->attachTo(this);

   spTextField text = new TextField();
   TextStyle style = TextStyle(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font"))
      .withColor(Color::White)
      .alignMiddle();
   style.fontSize = fontSize;
   text->setStyle(style);
   text->setColor(g_Layout.getPhosphorColor());
   text->setText("Drag to scroll, tap outside to continue");

   const Rect& rect = text->getTextRect();
   float padX = fontSize * 0.5f;
   float plateW = rect.size.x + 2.0f * padX;
   float plateH = rect.size.y + fontSize * 0.2f;

   spColorRectSprite plate = new ColorRectSprite();
   plate->setColor(Color(12, 20, 6, 255));
   plate->setAnchor(0.5f, 0.5f);
   plate->setSize(plateW, plateH);
   plate->setPosition(0.0f, 0.0f);
   plate->attachTo(m_tapLabel);

   // With middle alignment the text is centred on the field's rect
   text->setAnchor(0.0f, 0.0f);
   text->setSize(plateW, plateH);
   text->setPosition(-plateW / 2.0f, -plateH / 2.0f);
   text->attachTo(m_tapLabel);
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
   initMessage(leftBubble, messageString, senderString, getSenderAffiliation(senderString), preWait, postWait);
}

void MessageDisplay::initMessage(
   bool leftBubble,
   const char* messageString,
   const char* senderString,
   Affiliation affiliation,
   int preWait,
   int postWait)
{
   // The affiliation is taken now, so a message keeps its colour if the
   // sender changes side later
   MessageItem newMessage(messageString, senderString, leftBubble, affiliation, preWait, postWait);

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
      // we need to move old messages to make space for the new.
      // (State first: startTransit may finish at once and move on.)
      m_state = inTransit;

      startTransit();
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
      m_newBubble->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("msg_bbl_left"));
   }
   else
   {
      m_newBubble->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("msg_bbl_right"));
   }


   // The bubble texture is white; the frame shows the sender's affiliation
   m_newBubble->setColor(g_Layout.getAffiliationColor(message.m_affiliation));

   m_newBubble->setVerticalMode(Box9Sprite::STRETCHING);
   m_newBubble->setHorizontalMode(Box9Sprite::STRETCHING);

   if (message.m_leftBubble)
   {
      m_newBubble->setAnchor(0.0f, 1.0f);
      m_newBubble->setPosition(4.0f, m_messageDisplayHeight - c_clipInset - 4.0f);
   }
   else
   {
      m_newBubble->setAnchor(1.0f, 1.0f);
      m_newBubble->setPosition(m_messageDisplayWidth - 4.0f, m_messageDisplayHeight - c_clipInset - 4.0f);
   }

   spTextField msgTextField = new TextField();

   TextStyle style;
   style.multiline = true;
   style.vAlign = TextStyle::VerticalAlign::VALIGN_TOP;
   style.hAlign = TextStyle::HorizontalAlign::HALIGN_LEFT;

   style.font = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font");
   style.color = Color::White;
   style.fontSize = (int)m_fontSize;

   msgTextField->setStyle(style);
   msgTextField->setSize(Vector2(m_messageDisplayWidth - 40.0f - 8.0f, 0));
   msgTextField->setAnchor(0.0f, 0.0f);
   msgTextField->setPosition(8.0f, 8.0f);
   msgTextField->setText(message.m_message);
   if (c_textInAffiliationColour)
   {
      msgTextField->setColor(g_Layout.getAffiliationColor(message.m_affiliation));
   }
   else
   {
      msgTextField->setColor(g_Layout.getPhosphorColor());
   }

   const Rect& rect = msgTextField->getTextRect();

   m_newMessageHeight = rect.size.y + 20.0f;

   m_newBubble->setSize(rect.size.x + 20.0f, m_newMessageHeight);

   msgTextField->attachTo(m_newBubble);

}

void MessageDisplay::startTransit(void)
{
   // Now we know the size of the new entry. We can now move all other children 
   // up by that same amount

   // Bubbles that move out of view are kept (for scrolling back in the
   // expanded display); only the oldest beyond c_maxKeptMessages go
   pruneOldBubbles();

   // A new message brings the view back to the newest
   setScrollOffset(0.0f);

   spTween tween;

   spActor actor = m_scrollActor->getFirstChild();

   while (actor)
   {
      Vector2 pos = actor->getPosition();

      // Only those that can be seen are animated, the others just move
      if (pos.y >= 0)
      {
         tween = actor->addTween(Actor::TweenPosition(pos.x, pos.y - m_newMessageHeight - 4.0f), 500);
      }
      else
      {
         actor->setPosition(pos.x, pos.y - m_newMessageHeight - 4.0f);
      }

      actor = actor->getNextSibling();
   }

   if (tween)
   {
      // Lets use the last tween to get a call back when it is finished
      tween->setDoneCallback(CLOSURE(this, &MessageDisplay::atTransitFinished));
   }
   else
   {
      // Nothing visible to move; carry on right away
      atTransitFinished(NULL);
   }
}

void MessageDisplay::instantTransit(void)
{
   // Now we know the size of the new entry. We can now move all other children 
   // up by that same amount

   pruneOldBubbles();

   spActor actor = m_scrollActor->getFirstChild();

   while (actor)
   {
      Vector2 pos = actor->getPosition();
      actor->setPosition(pos.x, pos.y - m_newMessageHeight - 4.0f);
      actor = actor->getNextSibling();
   }
}

void MessageDisplay::startMessageAnimation(void)
{
   // When all old messages are pushed up, 
   // we add the new actors
   m_newBubble->setScale(0.0f);
   m_newBubble->attachTo(m_scrollActor);

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
      // On display now, so a redraw must include it; the queue entry
      // stays until the post wait is over
      m_displayedMessages.push_back(m_messageQueue[0]);
      m_ticks = 0;
      m_state = postWait;
   }
   else
   {
      m_state = idle;
      m_displayedMessages.push_back(*(m_messageQueue.begin()));
      m_messageQueue.erase(m_messageQueue.begin());
   }

   // The same number of messages as bubbles is kept for a redraw
   while (m_displayedMessages.size() > c_maxKeptMessages)
   {
      m_displayedMessages.erase(m_displayedMessages.begin());
   }
}

void MessageDisplay::redrawMessages(void)
{
   // A bubble that is still animating would be lost by the redraw (its
   // tween is on an actor that gets removed, so its done callback never
   // comes). Count it as displayed and let the queue carry on.
   if (((m_state == inTransit) || (m_state == newBubbleAnimation)) && !m_messageQueue.empty())
   {
      m_displayedMessages.push_back(m_messageQueue.front());
      m_messageQueue.erase(m_messageQueue.begin());
      m_state = idle;
   }

   // clearMessageDisplay also empties the queue, which a resize must not do
   std::vector<MessageItem> queue = m_messageQueue;
   clearMessageDisplay();
   m_messageQueue = queue;

   for (auto it = m_displayedMessages.begin(); it != m_displayedMessages.end(); ++it)
   {
      drawMessage(*it);

      if (m_noPreviousMessages)
      {
         m_noPreviousMessages = false;

         m_newBubble->setScale(1.0f);
         m_newBubble->attachTo(m_scrollActor);
      }
      else
      {
         // New message but there are already old messages on display
         // we need to move old messages to make space for the new
         instantTransit();
         m_newBubble->setScale(1.0f);
         m_newBubble->attachTo(m_scrollActor);
      }
   }
}

