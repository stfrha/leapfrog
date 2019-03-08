#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

class SceneActor;
class MessageDisplay;

DECLARE_SMART(MessageItem, spMessageItem);

class MessageItem : public oxygine::Sprite
{
public:
   enum MessageItemTypeEnum
   {
      me,
      friendlyMoving,
      friendlyStationary,
      neutralMoving,
      neutralStationary,
      enemyMoving,
      enemyStationary
   };

   enum MessageItemStateEnum
   {
      hollow,
      filled,
      flashingSlow,
      flashingFast
   };


private:
   oxygine::ResAnim* m_resAnim;
   MessageItemTypeEnum m_type;
   oxygine::spActor m_realActor;
   MessageDisplay* m_parentMap;
   int m_itemId;

public:
   MessageItem(
      oxygine::Resources* gameResources,
      MessageDisplay* hddMapActor,
      MessageItemTypeEnum type,
      MessageItemStateEnum state,
      int id,
      oxygine::spActor actor,
      float scale);

   MessageItemTypeEnum getType(void);
   oxygine::spActor getActor(void);
   int getId(void);
   void setState(MessageItem::MessageItemStateEnum state);

protected:
   void doUpdate(const oxygine::UpdateState& us);

};


DECLARE_SMART(MessageDisplay, spMessageDisplay);

class MessageDisplay : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResources;
   SceneActor* m_sceneActor;
   std::vector<spMessageItem> m_mapActors;
   float m_mapScale;
   float m_itemScale;
   int m_itemIdRepository;


public:
   float m_sceneWidth;
   float m_sceneHeight;


   MessageDisplay();

   void initialiseMessageDisplay(
      oxygine::Resources* gameResources,
      SceneActor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   void clearMessageDisplay(void);

   int addMeToMessageDisplay(
      MessageItem::MessageItemTypeEnum type,
      oxygine::spActor actor,
      MessageItem::MessageItemStateEnum state);

   void removeMeFromMessageDisplay(oxygine::spActor removeMe);
   void removeMeFromMessageDisplay(int itemId);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spMessageDisplay g_MessageDisplay;

