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
   oxygine::Resources* gameResources,
   HeadDownDisplay* hddMapActor,
   MapItemTypeEnum type,
   oxygine::spActor actor,
   float scale) :
   m_parentMap(hddMapActor)
{
   m_type = type;
   m_realActor = actor;

   switch (type)
   {
   case MapItemTypeEnum::me:
      setResAnim(gameResources->getResAnim("my_map_item"));
      break;
   case MapItemTypeEnum::friendlyMoving:
      setResAnim(gameResources->getResAnim("firendly_moving"));
      break;
   case MapItemTypeEnum::friendlyStationary:
      setResAnim(gameResources->getResAnim("friendly_stationary"));
      break;
   case MapItemTypeEnum::enemyMoving:
      setResAnim(gameResources->getResAnim("enemy_moving"));
      break;
   case MapItemTypeEnum::enemyStationary:
      setResAnim(gameResources->getResAnim("enemy_stationary"));
      break;
   case MapItemTypeEnum::neutralMoving:
      setResAnim(gameResources->getResAnim("neutral_moving"));
      break;
   case MapItemTypeEnum::neutralStationary:
      setResAnim(gameResources->getResAnim("neutral_stationary"));
      break;
   }

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
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


HeadDownDisplay::HeadDownDisplay()
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

   float xScale = (mapRight - mapLeft) / (bottomRight.x - topLeft.x);
   float yScale = (mapBottom - mapTop) / (bottomRight.y - topLeft.y);

   m_mapScale = fmin(xScale, yScale);
   m_itemScale = 1.0f / m_mapScale * g_Layout.getButtonWidth() / 80.0f;

   float thickness = 8.0f;

   setAnchor(0.5f, 0.5f);
   setSize(m_sceneWidth, m_sceneHeight);
   setPosition(mapCenter);
   setScale(m_mapScale);
   attachTo(sceneActor->getParent());

   spColorRectSprite theBar = new ColorRectSprite();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(m_sceneWidth, m_sceneHeight);
   theBar->setPosition(0.0f, 0.0f);
   theBar->setColor(Color::Fuchsia);
   theBar->setAlpha(128);
   theBar->attachTo(this);

   spColorRectSprite top = new ColorRectSprite();
   top->setColor(Color::Fuchsia);
   top->setAnchor(0.0f, 0.0f);
   top->setSize(m_sceneWidth, thickness);
   top->setPosition(0.0f, 0.0f);
   top->attachTo(theBar);

   spColorRectSprite bottom = new ColorRectSprite();
   bottom->setColor(Color::Fuchsia);
   bottom->setAnchor(0.0f, 0.0f);
   bottom->setSize(m_sceneWidth, thickness);
   bottom->setPosition(0.0f, bottomRight.y - thickness);
   bottom->attachTo(theBar);

   spColorRectSprite left = new ColorRectSprite();
   left->setColor(Color::Fuchsia);
   left->setAnchor(0.0f, 0.0f);
   left->setSize(thickness, m_sceneHeight);
   left->setPosition(0.0f, 0.0f);
   left->attachTo(theBar);

   spColorRectSprite right = new ColorRectSprite();
   right->setColor(Color::Fuchsia);
   right->setAnchor(0.0f, 0.0f);
   right->setSize(thickness, m_sceneHeight);
   right->setPosition(bottomRight.x - thickness, 0.0f);
   right->attachTo(theBar);

}

void HeadDownDisplay::addMeToMap(
   MapItem::MapItemTypeEnum type,
   spActor actor)
{
   spMapItem mi = new MapItem(
      m_gameResources,
      this,
      type,
      actor,
      m_itemScale);


   m_mapActors.push_back(mi);
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

void HeadDownDisplay::doUpdate(const oxygine::UpdateState& us)
{
}

