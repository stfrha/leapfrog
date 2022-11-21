#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

DECLARE_SMART(MessageDisplay, spMessageDisplay);

class MessageItem
{
public:
   std::string m_message;
   std::string m_sender;
   bool m_leftBubble;
   bool m_inTransit;
   int m_preWait;
   int m_postWait;

   MessageItem(
      std::string message,
      std::string sender,
      bool leftBubble,
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

   void clearMessageDisplay(void);


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

   void initMessage(
      bool leftBubble,
      const char* messageString,
      const char* senderString,
      int preWait,
      int postWait);

   void prepareFirstInQueue(void);
   void processFirstInQueue(void);
   void startMessageAnimation(void);
   void startTransit(void);

   void atTransitFinished(oxygine::Event* event);
   void atNewBubbleFinished(oxygine::Event* event);



protected:
	void doUpdate(const oxygine::UpdateState& us);

   // Put all displayed messages onto the display again, without
   // tweens. Used if size is changed.
   void redrawMessages(void);
   void drawMessage(const MessageItem& message);
   void instantTransit(void);

};

extern spMessageDisplay g_MessageDisplay;

