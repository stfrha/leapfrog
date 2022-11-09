#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "gamestatus.h"


class SceneActor;


DECLARE_SMART(PickupObject, spPickupObject);


// The PickupObject class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the PickupObject 
// Actor.

class PickupItem
{
public:
   enum PickupTypeEnum
   {
      notDef,
      resourceAmmo,
      resourceFuel,
      resourceShield,
      resourceCoin,
      inventory
   };

   enum PickupResourceAmountTypeEnum
   {
      notDefAmount,
      fix,
      random
   };

   PickupTypeEnum m_pickUpType;
   PickupResourceAmountTypeEnum m_pickupAmountType;
   float m_amount;
   float m_amountMax;
   float m_amountMin;
   std::string m_inventoryDescription;

   PickupItem( );

   PickupTypeEnum pickupTypeFromString(const std::string& s);
   PickupResourceAmountTypeEnum pickupAmountTypeFromString(const std::string& s);

   void readPickupObjectNode(const pugi::xml_node& node);

   void doPickupAmountCalculation(ObjectResources* status);
};

class PickupObject : public CompoundObject
{
private:
   b2World* m_world;
   SceneActor* m_sceneActor;

   PickupItem m_pickupItem;
   

public:
	PickupObject(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);
   
   void handlePickup(ObjectResources* status);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};
