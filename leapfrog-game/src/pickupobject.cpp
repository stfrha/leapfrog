
#include "pickupobject.h"
#include "pickupobjectevents.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"


using namespace std;
using namespace oxygine;
using namespace pugi;


PickupItem::PickupTypeEnum PickupItem::pickupTypeFromString(const string& s)
{
   if (s == "resourceAmmo") return PickupTypeEnum::resourceAmmo;
   if (s == "resourceFuel") return PickupTypeEnum::resourceFuel;
   if (s == "resourceShield") return PickupTypeEnum::resourceShield;
   if (s == "resourceCoin") return PickupTypeEnum::resourceCoin;
   if (s == "inventory") return PickupTypeEnum::inventory;
   return PickupTypeEnum::notDef;
}

PickupItem::PickupResourceAmountTypeEnum PickupItem::pickupAmountTypeFromString(const string& s)
{
   if (s == "fix") return PickupResourceAmountTypeEnum::fix;
   if (s == "random") return PickupResourceAmountTypeEnum::random;
   return PickupResourceAmountTypeEnum::notDefAmount;
}

void PickupItem::readPickupObjectNode(const pugi::xml_node& node)
{
   m_pickUpType = pickupTypeFromString(node.attribute("pickupType").as_string());
   m_pickupAmountType = pickupAmountTypeFromString(node.attribute("amountType").as_string());
   m_amount = node.attribute("amount").as_float(0);
   m_amountMax = node.attribute("amountMax").as_float(0);
   m_amountMin = node.attribute("amountMin").as_float(0);
   m_inventoryDescription = node.attribute("inventory").as_string();
}

PickupItem::PickupItem()
{
}

void PickupItem::doPickupAmountCalculation(GameStatus* status)
{
   if (m_pickUpType == PickupTypeEnum::inventory)
   {
      // TODO: Do nothing yet, later add inventory to game status
      return;
   }

   float amount = 0;

   if (m_pickupAmountType == PickupResourceAmountTypeEnum::fix)
   {
      amount = m_amount;
   }
   else
   {
      amount = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (m_amountMax - m_amountMin)) + m_amountMin);
   }
   
   if (m_pickUpType == PickupTypeEnum::resourceAmmo)
   {
      status->deltaShots(static_cast<int>(amount));
      return;
   }
   
   if (m_pickUpType == PickupTypeEnum::resourceShield)
   {
      status->deltaShield(amount);
      return;
   }

   if (m_pickUpType == PickupTypeEnum::resourceFuel)
   {
      status->deltaFuel(amount);
      return;
   }

   if (m_pickUpType == PickupTypeEnum::resourceCoin)
   {
      status->deltaCredits(static_cast<int>(amount));
      return;
   }
}

PickupObject::PickupObject(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_gameResource(&gameResources),
   m_sceneActor(sceneParent),
   m_world(world)
{
   m_pickupItem.readPickupObjectNode(root.child("behaviour").child("pickupObjectProperties"));
     
   initCompoundObjectParts(
      gameResources,
      sceneParent,
      sceneParent,
      parentObject,
      world,
      pos,
      root,
      groupIndex);

   attachTo(m_sceneActor);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   if (m_sceneActor->getSceneType() == SceneActor::SceneTypeEnum::deepSpace)
   {
      FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

      setAllBodiesToBounding(spaceActor);
   }

   // Give body a little nudge and pinch
   if (m_shapes.size() > 0)
   {
      b2Body* myBody = ActorUserData::getBody(m_shapes[0]->getUserData());

      if (myBody)
      {

         // Find magnitude of nudge and pinch by evaluating
         // the mass of the body

         float mass = myBody->GetMass();

         float maxImp = mass * 8.0f;
         float maxAng = mass * 6.0f;

         // Randomise value between 0 and 2pi
         float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f * MATH_PI));


         // Randomise value between 0 and maxImp
         float magnitude = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxImp));

         b2Vec2 impulseForce = b2Vec2(magnitude * cos(angle), magnitude * sin(angle));

         myBody->ApplyLinearImpulseToCenter(impulseForce, true);

         // Randomise value between 0 and maxAng
         float angImpulse = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxAng));

         myBody->ApplyAngularImpulse(angImpulse * 10.0f, true);
      }
   }
}



void PickupObject::handlePickup(GameStatus* status)
{
   m_pickupItem.doPickupAmountCalculation(status);

   m_isDead = true;
   addMeToDeathList();

}

void PickupObject::doUpdate(const oxygine::UpdateState& us)
{
   //if (m_spawnCount > 0)
   //{
   //   spawnPickupObjects();
   //}
}


