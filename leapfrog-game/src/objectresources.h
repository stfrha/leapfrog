#pragma once

#include "oxygine-framework.h"

DECLARE_SMART(ObjectResources, spObjectResources)

class ObjectProperty;
class Shield;

// Needs to be an ref_counter to send events
class ObjectResources : public oxygine::ref_counter
{
private:
   ObjectProperty* m_ammo;
   ObjectProperty* m_shield;
   ObjectProperty* m_fuel;
   ObjectProperty* m_credits;
   ObjectProperty* m_damage;

   Shield* m_shieldObj;

   // originators are used to dispatch events (since this global object is bad for being an actor)
   // and because (I presume) event goes down and back up a hiearchy branch which may differ
   // for different status views
   oxygine::Actor* m_statusEventOriginator;

public:
	ObjectResources();

   void initObjectResources(
      oxygine::Actor* originator,
      ObjectProperty* ammoProp,
      ObjectProperty* shieldProp,
      ObjectProperty* fuelProp,
      ObjectProperty* creditsProp,
      ObjectProperty* damageProp);

   void setResources(int ammo, float shield, float fuel, int credit, float damage);

   int getShots(void);
   void  deltaShots(int shots);

   float getShield(void);
   void deltaShield(float shield);
   void registerShieldObject(Shield* shieldObj);

   float getFuel(void);
   void deltaFuel(float fuel);

   int getCredits(void);
   void  deltaCredits(int credits);

   float getDamage(void);
   void deltaDamage(float damage);

};
