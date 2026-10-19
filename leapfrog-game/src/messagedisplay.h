#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "layout.h"

DECLARE_SMART(MessageDisplay, spMessageDisplay);

class MessageItem
{
public:
   std::string m_message;
   std::string m_sender;
   bool m_leftBubble;
   Affiliation m_affiliation;
   bool m_inTransit;
   int m_preWait;
   int m_postWait;

   MessageItem(
      std::string message,
      std::string sender,
      bool leftBubble,
      Affiliation affiliation,
      int preWait,
      int postWait);
};

class MessageDisplay : public oxygine::Box9Sprite
{
public:
   enum MsgDispStateEnum
   {
      idle,
      preWait,
      inTransit,
      newBubbleAnimation,
      postWait
   };

private:
   bool m_fullHeight; 

   oxygine::ClipRectActor* m_messageActor;
   // Holds the bubbles, inside the clip rect. Moved down to scroll back
   // to older messages in the expanded display.
   oxygine::Actor* m_scrollActor;
   float m_scrollOffset;
   bool m_dragging;
   float m_dragLastY;
   oxygine::Actor* m_mdFrame;

   float m_highTop;
   float m_normalTop;
   float m_left;
   float m_right;
   float m_highBottom;
   float m_normalBottom;

   float m_messageDisplayWidth;
   float m_messageDisplayHeight;

   float m_fontSize;

   std::vector<MessageItem> m_messageQueue;
   std::vector<MessageItem> m_displayedMessages;
   MsgDispStateEnum m_state;
   int m_ticks;
   bool m_noPreviousMessages;
   oxygine::spBox9Sprite m_newBubble;
   float m_newMessageHeight;

   // Tap to enlarge: the game pauses and the display is shown in the
   // middle of the screen, full height, wider and slightly zoomed in.
   // Dragging on it scrolls back through older messages; a tap outside
   // puts it back and continues the game.
   bool m_expanded;
   oxygine::spColorRectSprite m_dimmer;
   oxygine::spActor m_tapLabel;

   void clearMessageDisplay(void);
   void applySize(float width, float height);
   void touchDownHandler(oxygine::Event* event);
   void touchMoveHandler(oxygine::Event* event);
   void touchUpHandler(oxygine::Event* event);
   void wheelHandler(oxygine::Event* event);
   void dimmerTouchHandler(oxygine::Event* event);
   void setScrollOffset(float offset);
   float getMaxScrollOffset(void);
   void pruneOldBubbles(void);
   void expand(void);
   void collapse(bool animate);
   void collapseTweenDone(oxygine::Event* event);
   void createTapLabel(void);


public:
   MessageDisplay();

   void initialiseMessageDisplay(
      oxygine::Actor* parentActor,
      float highTop,
      float normalTop,
      float left,
      float right,
      float highBottom,
      float normalBottom,
      const float fontSize);

   void setFullHeight(bool fullHeight);

   void cleanAndClearMessageDisplay(void);

   // The bubble frame gets the colour of the sender's affiliation, as set
   // with setSenderAffiliation (none if the sender has not been set).
   void initMessage(
      bool leftBubble,
      const char* messageString,
      const char* senderString,
      int preWait,
      int postWait);

   // Same, with the affiliation given for this message only
   void initMessage(
      bool leftBubble,
      const char* messageString,
      const char* senderString,
      Affiliation affiliation,
      int preWait,
      int postWait);

   // Sender name -> affiliation. Kept for the whole run (not cleared with
   // the display), but not saved: missions set it up in
   // setupMissionStateScene, so it follows the mission state after a load.
   static void setSenderAffiliation(const std::string& sender, Affiliation affiliation);
   static Affiliation getSenderAffiliation(const std::string& sender);

   void prepareFirstInQueue(void);
   void processFirstInQueue(void);
   void startMessageAnimation(void);
   void startTransit(void);

   void atTransitFinished(oxygine::Event* event);
   void atNewBubbleFinished(oxygine::Event* event);



protected:
	void doUpdate(const oxygine::UpdateState& us) override;

   // Put all displayed messages onto the display again, without
   // tweens. Used if size is changed.
   void redrawMessages(void);
   void drawMessage(const MessageItem& message);
   void instantTransit(void);

};

extern spMessageDisplay g_MessageDisplay;

