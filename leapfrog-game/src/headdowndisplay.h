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

   enum MapItemStateEnum
   {
      hollow,
      filled,
      flashingSlow,
      flashingFast
   };


private:
   oxygine::ResAnim* m_resAnim;
   MapItemTypeEnum m_type;
   oxygine::spActor m_realActor;
   HeadDownDisplay* m_parentMap;
   MapItem::MapItemStateEnum m_state;
   int m_itemId;

public:
   // There are two constructors.
   // The first one is for 
   // creating map items without adding them 
   // to the map actor (just inserting them
   // in the collection of items). This is done when scene
   // files are read and all objects are created. At this
   // time there is no map yet. Some function needs to 
   // iterate the map item collection and add them all
   // to the map when it is created
   // The other is complete and used when obejcts are 
   // added at a later time, by the lua-script. At this
   // time the map actor will exist
   MapItem(
      MapItemTypeEnum type,
      MapItemStateEnum state,
      int id,
      oxygine::spActor actor);

   MapItem(
      oxygine::Resources* gameResources,
      HeadDownDisplay* hddMapActor,
      MapItemTypeEnum type,
      MapItemStateEnum state,
      int id,
      oxygine::spActor actor,
      float scale);

   void addToMapActor(
      oxygine::Resources* gameResources,
      HeadDownDisplay* hddMapActor,
      float scale);

   MapItemTypeEnum getType(void);
   oxygine::spActor getActor(void);
   int getId(void);
   void setState(MapItem::MapItemStateEnum state);

protected:
   void doUpdate(const oxygine::UpdateState& us);

};


DECLARE_SMART(HeadDownDisplay, spHeadDownDisplay);

class HeadDownDisplay : public oxygine::Actor
{
private:
   bool m_actorExists;
   oxygine::Resources* m_gameResources;
   SceneActor* m_sceneActor;
   std::vector<spMapItem> m_mapActors;
   float m_mapScale;
   float m_itemScale;
   int m_itemIdRepository;


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

   int addMeToMap(
      MapItem::MapItemTypeEnum type,
      oxygine::spActor actor,
      MapItem::MapItemStateEnum state);

   void removeMeFromMap(oxygine::spActor removeMe);
   void removeMeFromMap(int itemId);

   // Fast version, caller must have remembered the item idenitfier.
   void setState(int itemId, MapItem::MapItemStateEnum state);

   // Slow version, the item is looked up
   void setState(oxygine::spActor actor, MapItem::MapItemStateEnum state);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spHeadDownDisplay g_HeadDownDisplay;

