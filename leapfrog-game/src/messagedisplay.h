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
   int m_preWait;
   int m_postWait;

   MessageItem(
      std::string message,
      std::string sender,
      bool leftBubble,
      int preWait,
      int postWait);
};

class MessageDisplay : public oxygine::Actor
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
   oxygine::Resources* m_hudResources;
   oxygine::spClipRectActor m_messageActor;

   float m_messageDisplayWidth;
   float m_messageDisplayHeight;
   float m_fontSize;

   std::vector<MessageItem> m_messageQueue;
   MsgDispStateEnum m_state;
   int m_ticks;
   bool m_noPreviousMessages;
   oxygine::spBox9Sprite m_newBubble;
   float m_newMessageHeight;

public:
   MessageDisplay();

   void initialiseMessageDisplay(
      oxygine::Resources* hudResources,
      oxygine::Actor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight,
      const float fontSize);

   void clearMessageDisplay(void);

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
};

extern spMessageDisplay g_MessageDisplay;

