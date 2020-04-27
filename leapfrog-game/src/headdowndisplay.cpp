#include "headdowndisplay.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spHeadDownDisplay g_HeadDownDisplay;

MapItem::MapItem(
   MapItemTypeEnum type,
   MapItemStateEnum state,
   int id,
   oxygine::spActor actor) :
   m_itemId(id),
   m_type(type),
   m_state(state),
   m_parentMap(NULL),
   m_realActor(actor)
{
   int a = 10;
}


MapItem::MapItem(
   oxygine::Resources* gameResources,
   HeadDownDisplay* hddMapActor,
   MapItemTypeEnum type,
   MapItemStateEnum state,
   int id,
   oxygine::spActor actor,
   float scale) :
   m_itemId(id),
   m_type(type),
   m_state(state),
   m_parentMap(hddMapActor),
   m_realActor(actor)
{
   switch (m_type)
   {
   case MapItemTypeEnum::me:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = gameResources->getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = gameResources->getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = gameResources->getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = gameResources->getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = gameResources->getResAnim("neutral_stationary");
      break;
   }

   setState(m_state);

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
   setAlpha(128);
   setScale(scale);
   attachTo(hddMapActor);
}

void MapItem::addToMapActor(
   oxygine::Resources* gameResources,
   HeadDownDisplay* hddMapActor,
   float scale)
{
   m_parentMap = hddMapActor;

   switch (m_type)
   {
   case MapItemTypeEnum::me:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = gameResources->getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = gameResources->getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = gameResources->getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = gameResources->getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = gameResources->getResAnim("neutral_stationary");
      break;
   }

   setState(m_state);

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
   setAlpha(128);
   setScale(scale);
   attachTo(hddMapActor);
}


MapItem::MapItemTypeEnum MapItem::getType(void)
{
   return m_type;
}

oxygine::spActor MapItem::getActor(void)
{
   return m_realActor;
}

int MapItem::getId(void)
{
   return m_itemId;
}

void MapItem::setState(MapItem::MapItemStateEnum state)
{
   switch (state)
   {
   case hollow:
      removeTweens();
      setResAnim(m_resAnim, 0, 0);
      break;
   case filled:
      removeTweens();
      setResAnim(m_resAnim, 1, 0);
      break;
   case flashingSlow:
      setResAnim(m_resAnim, 0, 0);
      removeTweens();
      addTween(TweenAnim(m_resAnim), 1000, -1);
      break;
   case flashingFast:
      setResAnim(m_resAnim, 0, 0);
      removeTweens();
      addTween(TweenAnim(m_resAnim), 250, -1);
      break;
   }
}


void MapItem::doUpdate(const oxygine::UpdateState& us)
{
   Vector2 pos = m_realActor->getPosition();

   if (pos.x < 0.0f)
   {
      pos.x = 0.0f;
   }

   if (pos.y < 0.0f)
   {
      pos.y = 0.0f;
   }

   if (pos.x > m_parentMap->m_sceneWidth)
   {
      pos.x = m_parentMap->m_sceneWidth;
   }

   if (pos.y > m_parentMap->m_sceneHeight)
   {
      pos.y = m_parentMap->m_sceneHeight;
   }

   setPosition(pos);
}


HeadDownDisplay::HeadDownDisplay() :
   m_itemIdRepository(0),
   m_actorExists(false)
{
}

void HeadDownDisplay::clearMap(void)
{
   // since this is class is supposed to be instance once globally
   // (a lazy singleton implementation) we need to clean up all here
   // and recreate all 
   // Clean up current scene
   spActor actor = getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
         actor->detach();
      }
      actor = next;
   }

   // vector is of type spActor so clearing it will delete all items
   m_mapActors.clear();

   m_actorExists = false;

}


void HeadDownDisplay::initialiseMap(
   Resources* gameResources,
   SceneActor* sceneActor,
   const Vector2& topLeft, 
   const Vector2& bottomRight)
{
   m_gameResources = gameResources;
   m_sceneActor = sceneActor;

   // The map is the same size as the game map but scaled to fit into the 
   // map window between the lower buttons

   m_sceneWidth = bottomRight.x - topLeft.x;
   m_sceneHeight = bottomRight.y - topLeft.y;

   // Calculate the rect where the map is to be
   float mapLeft = g_Layout.getXFromLeft(2) + g_Layout.getButtonWidth() / 4.0f;
   float mapRight = g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 4.0f;
   float mapTop = g_Layout.getYFromBottom(1);
   float mapBottom = g_Layout.getYFromBottom(-1) - g_Layout.getButtonWidth() / 4.0f;
   Vector2 mapCenter((mapRight - mapLeft) / 2.0f + mapLeft, (mapBottom - mapTop) / 2.0f + mapTop);

   float xScale = (mapRight - mapLeft) / (m_sceneWidth);
   float yScale = (mapBottom - mapTop) / (m_sceneHeight);

   m_mapScale = fmin(xScale, yScale);
   m_itemScale = 1.0f / m_mapScale * g_Layout.getButtonWidth() / 80.0f;

   float thickness = 8.0f;

   setAnchor(0.5f, 0.5f);
   setSize(m_sceneWidth, m_sceneHeight);
   setPosition(mapCenter);
   setScale(m_mapScale);
   attachTo(sceneActor->getParent());

   spBox9Sprite hej = new Box9Sprite();
   hej->setResAnim(m_gameResources->getResAnim("display_fat"));
   hej->setVerticalMode(Box9Sprite::STRETCHING);
   hej->setHorizontalMode(Box9Sprite::STRETCHING);
   hej->setSize(m_sceneWidth, m_sceneHeight);
   hej->setAnchor(0.0f, 0.0f);
   hej->setPosition(0.0f, 0.0f);
   hej->setGuides(8, 120, 8, 120);
   hej->setTouchEnabled(false);
   hej->attachTo(this);

   m_actorExists = true;

   // Some map items has already been added as the xml-file was read
   // but they are not yet attached to the map (since it was just
   // created, how could they?). Now it is time to add them.
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      (*it)->addToMapActor(m_gameResources, this, m_itemScale);
   }
}

int HeadDownDisplay::addMeToMap(
   MapItem::MapItemTypeEnum type,
   spActor actor,
   MapItem::MapItemStateEnum state)
{
   // Check that this actor has not been added before.
   // If so, just change state.

   // We need to know if the map actor exists 
   // or not. 
   spMapItem mi;
   if (m_actorExists)
   {
       mi = new MapItem(
         m_gameResources,
         this,
         type,
         state,
         m_itemIdRepository,
         actor,
         m_itemScale);
   }
   else
   {
      mi = new MapItem(type, state, m_itemIdRepository, actor);
   }

   m_mapActors.push_back(mi);

   return m_itemIdRepository++;
}

void HeadDownDisplay::removeMeFromMap(spActor removeMe)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getActor() == removeMe)
      {
         spActor d = (*it)->getActor();
         (*it)->detach();
         m_mapActors.erase(it);
         return;
      }
   }
}


void HeadDownDisplay::removeMeFromMap(int itemId)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getId() == itemId)
      {
         spActor d = (*it)->getActor();
         (*it)->detach();
         m_mapActors.erase(it);
         return;
      }
   }
}

void HeadDownDisplay::setState(int itemId, MapItem::MapItemStateEnum state)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getId() == itemId)
      {
         (*it)->setState(state);
         return;
      }
   }
}

void HeadDownDisplay::setState(oxygine::spActor actor, MapItem::MapItemStateEnum state)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getActor() == actor)
      {
         (*it)->setState(state);
         return;
      }
   }
}


void HeadDownDisplay::doUpdate(const oxygine::UpdateState& us)
{
}

