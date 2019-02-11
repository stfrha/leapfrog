#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

class SceneActor;
class HeadDownDisplay;

DECLARE_SMART(MapItem, spMapItem);

class MapItem : public oxygine::Sprite
{
public:
   enum MapItemTypeEnum
   {
      me,
      friendlyMoving,
      friendlyStationary,
      neutralMoving,
      neutralStationary,
      enemyMoving,
      enemyStationary
   };


private:
   MapItemTypeEnum m_type;
   oxygine::spActor m_realActor;
   HeadDownDisplay* m_parentMap;

public:
   MapItem(
      oxygine::Resources* gameResources,
      HeadDownDisplay* hddMapActor,
      MapItemTypeEnum type,
      oxygine::spActor actor,
      float scale);

   MapItemTypeEnum getType(void);
   oxygine::spActor getActor(void);

protected:
   void doUpdate(const oxygine::UpdateState& us);

};


DECLARE_SMART(HeadDownDisplay, spHeadDownDisplay);

class HeadDownDisplay : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResources;
   SceneActor* m_sceneActor;
   std::vector<spMapItem> m_mapActors;
   float m_mapScale;
   float m_itemScale;


public:
   float m_sceneWidth;
   float m_sceneHeight;


   HeadDownDisplay();

   void initialiseMap(
      oxygine::Resources* gameResources,
      SceneActor* sceneActor,
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   void clearMap(void);

   void addMeToMap(
      MapItem::MapItemTypeEnum type,
      oxygine::spActor actor);

   void removeMeFromMap(oxygine::spActor removeMe);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spHeadDownDisplay g_HeadDownDisplay;

