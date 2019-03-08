#include "messagedisplay.h"

#include "sceneactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spMessageDisplay g_MessageDisplay;

MessageItem::MessageItem(
   oxygine::Resources* gameResources,
   MessageDisplay* hddMapActor,
   MessageItemTypeEnum type,
   MessageItemStateEnum state,
   int id,
   oxygine::spActor actor,
   float scale) :
   m_itemId(id),
   m_type(type),
   m_parentMap(hddMapActor),
   m_realActor(actor)

{
   switch (type)
   {
   case MessageItemTypeEnum::me:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MessageItemTypeEnum::friendlyMoving:
      m_resAnim = gameResources->getResAnim("friendly_moving");
      break;
   case MessageItemTypeEnum::friendlyStationary:
      m_resAnim = gameResources->getResAnim("friendly_stationary");
      break;
   case MessageItemTypeEnum::enemyMoving:
      m_resAnim = gameResources->getResAnim("enemy_moving");
      break;
   case MessageItemTypeEnum::enemyStationary:
      m_resAnim = gameResources->getResAnim("enemy_stationary");
      break;
   case MessageItemTypeEnum::neutralMoving:
      m_resAnim = gameResources->getResAnim("neutral_moving");
      break;
   case MessageItemTypeEnum::neutralStationary:
      m_resAnim = gameResources->getResAnim("neutral_stationary");
      break;
   }

   setState(state);

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
   setAlpha(128);
   setScale(scale);
   attachTo(hddMapActor);
}

MessageItem::MessageItemTypeEnum MessageItem::getType(void)
{
   return m_type;
}

oxygine::spActor MessageItem::getActor(void)
{
   return m_realActor;
}

int MessageItem::getId(void)
{
   return m_itemId;
}

void MessageItem::setState(MessageItem::MessageItemStateEnum state)
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


void MessageItem::doUpdate(const oxygine::UpdateState& us)
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


MessageDisplay::MessageDisplay() :
   m_itemIdRepository(0)
{
}

void MessageDisplay::clearMessageDisplay(void)
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


void MessageDisplay::initialiseMessageDisplay(
   Resources* gameResources,
   SceneActor* sceneActor,
   const Vector2& topLeft, 
   const Vector2& bottomRight)
{
   m_gameResources = gameResources;
   m_sceneActor = sceneActor;

   // The MessageDisplay is the same size as the game MessageDisplay but scaled to fit into the 
   // MessageDisplay window between the lower buttons


   // Calculate the rect where the MessageDisplay is to be
   float mapLeft = 0.0f;
   float mapRight = 160.0f;
   float mapTop = g_Layout.getButtonWidth();
   float mapBottom = g_Layout.getYFromBottom(2);

   m_sceneWidth = mapRight - mapLeft;
   m_sceneHeight = mapBottom - mapTop;

   Vector2 mapCenter((m_sceneWidth) / 2.0f + mapLeft, (m_sceneHeight) / 2.0f + mapTop);

   //float xScale = (mapRight - mapLeft) / (bottomRight.x - topLeft.x);
   //float yScale = (mapBottom - mapTop) / (bottomRight.y - topLeft.y);

   //m_mapScale = fmin(xScale, yScale);
   //m_itemScale = 1.0f / m_mapScale * g_Layout.getButtonWidth() / 80.0f;

   m_mapScale = 1.0f;

   float thickness = 4.0f;

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
   theBar->setAlpha(32);
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
   bottom->setPosition(0.0f, m_sceneHeight - thickness);
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
   right->setPosition(m_sceneWidth - thickness, 0.0f);
   right->attachTo(theBar);

}

int MessageDisplay::addMeToMessageDisplay(
   MessageItem::MessageItemTypeEnum type,
   spActor actor,
   MessageItem::MessageItemStateEnum state)
{

   spMessageItem mi = new MessageItem(
      m_gameResources,
      this,
      type,
      state,
      m_itemIdRepository,
      actor,
      m_itemScale);


   m_mapActors.push_back(mi);

   return m_itemIdRepository++;
}

void MessageDisplay::removeMeFromMessageDisplay(spActor removeMe)
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


void MessageDisplay::removeMeFromMessageDisplay(int itemId)
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

void MessageDisplay::doUpdate(const oxygine::UpdateState& us)
{
}

