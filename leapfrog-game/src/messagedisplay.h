#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

class SceneActor;

DECLARE_SMART(MessageDisplay, spMessageDisplay);

class MessageDisplay : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResources;
   oxygine::spClipRectActor m_messageActor;

   float m_messageDisplayWidth;
   float m_messageDisplayHeight;

public:
   MessageDisplay();

   void initialiseMessageDisplay(
      oxygine::Resources* gameResources,
      SceneActor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   void clearMessageDisplay(void);

   void initMessage(
      bool leftBubble,
      std::string& messageString,
      std::string& senderString);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spMessageDisplay g_MessageDisplay;

