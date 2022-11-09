#include "headdowndisplay.h"

#include "mainactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"
#include "graphicresources.h"

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
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_stationary");
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
   HeadDownDisplay* hddMapActor,
   float scale)
{
   m_parentMap = hddMapActor;

   switch (m_type)
   {
   case MapItemTypeEnum::me:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_stationary");
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
   m_actorExists(false),
   m_hudMode(HudModeType::hudModeMap)
{
}

void HeadDownDisplay::clearDisplay(void)
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

}

void HeadDownDisplay::clearMap(void)
{
   clearDisplay();

   // vector is of type spActor so clearing it will delete all items
   m_mapActors.clear();

   // m_actorExists = false;

}


void HeadDownDisplay::initialiseHdd(
   MainActor* mainActor,
   const Vector2& topLeft, 
   const Vector2& bottomRight)
{
   m_mainActor = mainActor;

   // The HUD can work as game menu, map or orbit overlay. The modes 
   // all have a common ancor on the screen, at the midde bottom. 
   // The game menu and orbit overlay have the top margin the same as the bottom.
   // The orbit overlay goes from left = 0 to right = screen width
   // The game menu goes from TBD to TBD
   // The map is the same size as the game map but scaled to fit into the 
   // map window between the lower buttons
   // In map modes, the size of the map needs to be the same as the scene 
   // but scaled to the map size. This is because that makes the position
   // of map items on the map the same as the real positions in the scene
   // However, when using the orbit overlay or game menu, we want the scale
   // to one. 
   // The animation of the size transitions is done in screen coordinates.
   // so, when shrinking to map size, this is done in scale 1, using the 
   // size as animation. When the map animation is finished, the size and scale
   // is changed in the same step, doing a seamless switch to map scale.

   m_sceneWidth = bottomRight.x - topLeft.x;
   m_sceneHeight = bottomRight.y - topLeft.y;

   // Calculate the rect where the map is to be
   float mapLeft = g_Layout.getXFromLeft(2) + g_Layout.getButtonWidth() / 4.0f;
   float mapRight = g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 4.0f;
   float mapTop = g_Layout.getYFromBottom(1);
   float mapBottom = g_Layout.getYFromBottom(-1) - g_Layout.getButtonWidth() / 4.0f;
   // Vector2 mapPos((mapRight - mapLeft) / 2.0f + mapLeft, (mapBottom - mapTop) / 2.0f + mapTop);
   Vector2 mapPos((mapRight - mapLeft) / 2.0f + mapLeft, mapBottom);

   float mapScaleX = (mapRight - mapLeft) / (m_sceneWidth);
   float mapScaleY = (mapBottom - mapTop) / (m_sceneHeight);

   m_mapScale = fmin(mapScaleX, mapScaleY);
   m_mapSizeX = m_sceneWidth * m_mapScale;
   m_mapSizeY = m_sceneHeight * m_mapScale;

   m_itemScale = 1.0f / m_mapScale * g_Layout.getButtonWidth() / 80.0f;

   m_menuSizeX = g_Layout.getViewPortBounds().x - 4.0f * g_Layout.getButtonWidth()  - 0.2f * g_Layout.getButtonWidth();
   m_menuSizeY = g_Layout.getViewPortBounds().y - 2.0f * (g_Layout.getViewPortBounds().y - mapBottom);

   m_orbitSizeX = g_Layout.getViewPortBounds().x;
   m_orbitSizeY = m_menuSizeY;

   float thickness = 8.0f;
   
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_fat"));
   setVerticalMode(Box9Sprite::STRETCHING);
   setHorizontalMode(Box9Sprite::STRETCHING);
   setSize(m_sceneWidth, m_sceneHeight);
   setAnchor(0.5f, 1.0f);
   setPosition(mapPos);
   setScale(m_mapScale);
   setGuides(8, 120, 8, 120);
   setTouchEnabled(false);
   attachTo(mainActor);

   m_actorExists = true;

   // Some map items has already been added as the xml-file was read
   // but they are not yet attached to the map (since it was just
   // created, how could they?). Now it is time to add them.
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      (*it)->addToMapActor(this, m_itemScale);
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

void HeadDownDisplay::goToOrbit(void)
{
   if (m_hudMode == HudModeType::hudModeMap)
   {
      clearDisplay();
      setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
      setSize(m_mapSizeX, m_mapSizeY);
      setScale(1.0f);
   }
   m_hudMode = HudModeType::hudModeOrbit;
   spTween tween = addTween(Actor::TweenSize(m_orbitSizeX, m_orbitSizeY), 500);
   tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::menuStartTransitionComplete));
}

void HeadDownDisplay::goToMenu(void)
{
   if (m_hudMode == HudModeType::hudModeMap)
   {
      clearDisplay();
      setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
      setSize(m_mapSizeX, m_mapSizeY);
      setScale(1.0f);
   }
   m_hudMode = HudModeType::hudModeMenu;
   spTween tween = addTween(Actor::TweenSize(m_menuSizeX, m_menuSizeY), 750, 1, false, 0, oxygine::Tween::EASE::ease_outBounce);
   tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::menuStartTransitionComplete));
}

void HeadDownDisplay::goToMap(void)
{
   if (m_hudMode != HudModeType::hudModeMap)
   {
      clearDisplay();
      spTween tween = addTween(Actor::TweenSize(m_mapSizeX, m_mapSizeY), 500);
      tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::mapTransitionComplete));

   }

}

void HeadDownDisplay::menuStartTransitionComplete(Event* event)
{
   // TODO: Light up buttons (in mainactor) only at this time
   m_mainActor->menuStartTransitionComplete();
}


void HeadDownDisplay::mapTransitionComplete(oxygine::Event* event)
{
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_fat"));
   setSize(m_sceneWidth, m_sceneHeight);
   setScale(m_mapScale);
   m_hudMode = HudModeType::hudModeMap;

   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      (*it)->addToMapActor(this, m_itemScale);
   }

   m_mainActor->restartedFromMenu();
}
