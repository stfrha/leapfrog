
#include "pickupobject.h"
#include "pickupobjectevents.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"
#include "headdowndisplay.h"


using namespace std;
using namespace oxygine;
using namespace pugi;


PickupItem::PickupTypeEnum PickupItem::pickupTypeFromString(const string& s)
{
   if (s == "resourceAmmo") return PickupTypeEnum::resourceAmmo;
   if (s == "resourceFuel") return PickupTypeEnum::resourceFuel;
   if (s == "resourceShield") return PickupTypeEnum::resourceShield;
   if (s == "resourceCoin")
   {
      return PickupTypeEnum::resourceCoin;

   }
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

void PickupItem::doPickupAmountCalculation(ObjectResources* status)
{
   // Only the Leapfrog's own pickups go to the HDD log, not those
   // made by other steerable objects
   bool logIt = (status == g_GameStatus.getResources());

   if (m_pickUpType == PickupTypeEnum::inventory)
   {
      // TODO: Do nothing yet, later add inventory to game status
      if (logIt && !m_inventoryDescription.empty())
      {
         g_HeadDownDisplay->logMessage("Picked up " + m_inventoryDescription);
      }
      return;
   }

   float amount = 0;

   if (m_pickupAmountType == PickupResourceAmountTypeEnum::fix)
   {
      amount = m_amount;
   }
   else
   {
      // amount = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (m_amountMax - m_amountMin)) + m_amountMin);
      amount = static_cast<float>(rand() / static_cast<float>(RAND_MAX + 1)) * (m_amountMax - m_amountMin + 1.0f) + m_amountMin;
   }
   
   // The resources are capped, so the log reports what was actually
   // gained rather than the nominal amount of the pickup
   if (m_pickUpType == PickupTypeEnum::resourceAmmo)
   {
      int before = status->getShots();
      status->deltaShots(static_cast<int>(amount));
      if (logIt)
      {
         int gained = status->getShots() - before;
         if (gained > 0) g_HeadDownDisplay->logAmount("ammo", "Picked up %d ammo", gained);
         else g_HeadDownDisplay->logMessage("Ammo full");
      }
      return;
   }
   
   if (m_pickUpType == PickupTypeEnum::resourceShield)
   {
      float before = status->getShield();
      status->deltaShield(amount);
      if (logIt)
      {
         int gained = static_cast<int>(status->getShield() - before + 0.5f);
         if (gained > 0) g_HeadDownDisplay->logAmount("shield", "Shield recharged %d", gained);
         else g_HeadDownDisplay->logMessage("Shield full");
      }
      return;
   }

   if (m_pickUpType == PickupTypeEnum::resourceFuel)
   {
      float before = status->getFuel();
      status->deltaFuel(amount);
      if (logIt)
      {
         int gained = static_cast<int>(status->getFuel() - before + 0.5f);
         if (gained > 0) g_HeadDownDisplay->logAmount("fuel", "Picked up %d fuel", gained);
         else g_HeadDownDisplay->logMessage("Fuel tank full");
      }
      return;
   }

   if (m_pickUpType == PickupTypeEnum::resourceCoin)
   {
      status->deltaCredits(static_cast<int>(amount));
      if (logIt)
      {
         g_HeadDownDisplay->logAmount("credits", "Picked up %d credits", static_cast<int>(amount));
      }
      return;
   }
}

PickupObject::PickupObject(
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_sceneActor(sceneParent),
   m_world(world)
{
   m_pickupItem.readPickupObjectNode(root.child("behaviour").child("pickupObjectProperties"));
     
   initCompoundObjectParts(
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



void PickupObject::handlePickup(ObjectResources* status)
{
   m_pickupItem.doPickupAmountCalculation(status);

   PickedUpEvent event;
   dispatchEvent(&event);

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


