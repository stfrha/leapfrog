#include "gamestatus.h"
#include "gamestatusevents.h"

#include "objectproperty.h"
#include "shield.h"

using namespace oxygine;


// Game status holds the resources (and should thus be renamed) to ObjectResources
// There is a smart-pointer to it in each CompoundObject (m_gameStatus). It is initialized
// with the initGameStatus(spGameStatus status) method (in CompoundObject) and the object 
// must, thus be created outside the CompoundObject. (Is this a good design? Why not just
// have it as a part of the CompoundObject?) 
// The GameStatus class can send events if configured to do so. It is done by calling
// the initGameStatus(Actor* eventOriginator) whereby the m_statusEventOriginator member
// is set to something other than NULL. Events are dispatched from this originator actor
// (which usually is the main actor). 
// TODO: THe LUA script want to set up triggers for the resources in the same way as any
// property. A few possible solutions are available for this:
// 1. The game status values are added as properties of the CompoundObject. The properties
//    are only added to those objects that needs and uses them. They are added by the 
//    specialisation of the CompoundObject. 
// 2. The game status has its own properties (must be a CompoundObject in that case) and
//    becomes an virtual actor that can be identified by the string: "objectname.status".
// Lets go for 1. This means that the status properties get indices in the list of properties
// of the object. How to access the properties and the status then?
// One way is by using the LUA function c_setObjectProperty. This is used by the script,
// for instance to award credits after mission complete. Another way is by the using the 
// built-in methods of the GameStatus class: deltaShots, deltaShield etc. 
// The difference between today is that the actual memeber variable that holds the resource
// is changed to a pointer to a ObjectProperty. The property is created in the CompoundObject
// specialisation and pointers ofd the properties are provided to the game status class.



GameStatus::GameStatus() :
   m_statusEventOriginator(NULL),
   m_ammo(NULL),
   m_shield(NULL),
   m_fuel(NULL),
   m_credits(NULL),
   m_damage(NULL)
{
}


void GameStatus::initGameStatus(
   oxygine::Actor* originator,
   ObjectProperty* ammoProp,
   ObjectProperty* shieldProp,
   ObjectProperty* fuelProp,
   ObjectProperty* creditsProp,
   ObjectProperty* damageProp)
{
   m_statusEventOriginator = originator;
   m_ammo = ammoProp;
   m_shield = shieldProp;
   m_fuel = fuelProp;
   m_credits = creditsProp;
   m_damage = damageProp;

   // This should probably be read from a file or 
   // some non-volatile memory. But how?!?!
   if (m_ammo != NULL) m_ammo->setProperty(100.0f);
   if (m_fuel != NULL) m_fuel->setProperty(100.0f);
   if (m_shield != NULL) m_shield->setProperty(100.0f);
   if (m_damage != NULL) m_damage->setProperty(0.0f);
   if (m_credits != NULL) m_credits->setProperty(20);
}


int GameStatus::getShots(void)
{
   return (int)m_ammo->getProperty();
}

void  GameStatus::deltaShots(int deltaShots)
{
   int ammo = (int)m_ammo->getProperty();

   ammo += deltaShots;

   if (ammo > 100)
   {
      ammo = 100;
   }

   if (m_statusEventOriginator)
   {
      //StatusChangedEvent event(GameStatusTypeEnum::shots, m_ammo);
      //m_statusEventOriginator->dispatchEvent(&event);

      if (ammo <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::shots);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (ammo < 0)
   {
      ammo = 0;
   }

   m_ammo->setProperty((float)ammo);
}

float GameStatus::getShield(void)
{
   return m_shield->getProperty();
}

void GameStatus::deltaShield(float deltaShield)
{
   float shield = m_shield->getProperty();

   if ((shield == 0) && (deltaShield > 0))
   {
      m_shieldObj->restartShield();
   }

   shield += deltaShield;

   if (shield > 100)
   {
      shield = 100;
   }

   if (m_statusEventOriginator)
   {
      //StatusChangedEvent event(GameStatusTypeEnum::shield, m_shield);
      //m_statusEventOriginator->dispatchEvent(&event);

      if (shield <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::shield);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (shield < 0.0f)
   {
      shield = 0.0f;
   }

   m_shield->setProperty(shield);

}


void GameStatus::registerShieldObject(Shield* shieldObj)
{
   m_shieldObj = shieldObj;
}


float GameStatus::getFuel(void)
{
   return m_fuel->getProperty();
}

void GameStatus::deltaFuel(float deltaFuel)
{
   float fuel = m_fuel->getProperty();

   fuel += deltaFuel;

   if (fuel > 100)
   {
      fuel = 100;
   }

   if (m_statusEventOriginator)
   {
      //StatusChangedEvent event(GameStatusTypeEnum::fuel, m_fuel);
      //m_statusEventOriginator->dispatchEvent(&event);

      if (fuel <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::fuel);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (fuel < 0.0f)
   {
      fuel = 0.0f;
   }

   m_fuel->setProperty(fuel);
}

int GameStatus::getCredits(void)
{
   return m_credits->getProperty();
}

void  GameStatus::deltaCredits(int deltaCredits)
{
   int credits = (int)m_credits->getProperty();

   credits += deltaCredits;

   if (m_statusEventOriginator)
   {
      //StatusChangedEvent event(GameStatusTypeEnum::credits, m_credits);
      //m_statusEventOriginator->dispatchEvent(&event);

      if (credits <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::credits);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   m_credits->setProperty(credits);
}

float GameStatus::getDamage(void)
{
   return m_damage->getProperty();
}

void GameStatus::deltaDamage(float deltaDamage)
{
   float damage = m_damage->getProperty();

   damage += deltaDamage;

   if (m_statusEventOriginator)
   {
      //StatusChangedEvent event(GameStatusTypeEnum::damage, m_damage);
      //m_statusEventOriginator->dispatchEvent(&event);

      if (damage <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::damage);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (damage < 0.0f)
   {
      damage = 0.0f;
   }

   m_damage->setProperty(damage);
}

