#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

class SceneActor;

DECLARE_SMART(MessageDisplay, spMessageDisplay);

class MessageItem
{
public:
   std::string m_message;
   std::string m_sender;
   bool m_leftBubble;
   bool m_inTransit;

   MessageItem(
      std::string message,
      std::string sender,
      bool leftBubble);
};

class MessageDisplay : public oxygine::Actor
{
public:
   enum MsgDispStateEnum
   {
      idle,
      inTransit,
      newBubbleAnimation
   };

private:
   oxygine::Resources* m_gameResources;
   oxygine::spClipRectActor m_messageActor;

   float m_messageDisplayWidth;
   float m_messageDisplayHeight;

   std::vector<MessageItem> m_messageQueue;
   MsgDispStateEnum m_state;
   bool m_noMessagesYet;
   oxygine::spBox9Sprite m_newBubble;
   float m_newMessageHeight;

public:
   MessageDisplay();

   void initialiseMessageDisplay(
      oxygine::Resources* gameResources,
      oxygine::Actor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   void clearMessageDisplay(void);

   void initMessage(
      bool leftBubble,
      const char* messageString,
      const char* senderString);

   void processFirstInQueue(void);
   void startMessageAnimation(void);
   void startTransit(void);

   void atTransitFinished(oxygine::Event* event);
   void atNewBubbleFinished(oxygine::Event* event);



protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spMessageDisplay g_MessageDisplay;

