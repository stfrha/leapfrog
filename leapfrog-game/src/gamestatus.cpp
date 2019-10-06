#include "gamestatus.h"
#include "gamestatusevents.h"

#include "shield.h"

using namespace oxygine;

GameStatus::GameStatus()
{
   // This should probably be read from a file or 
   // some non-volatile memory. But how?!?!
   m_ammo = 100;
   m_fuel = 100.0f;
   m_shield = 100.0f;
   m_damage = 0.0f;
   m_credits = 248;
   m_statusEventOriginator = NULL;
}

void GameStatus::initGameStatus(Actor* statusEventOriginator)
{
   m_statusEventOriginator = statusEventOriginator;
}

int GameStatus::getShots(void)
{
   return m_ammo;
}

void  GameStatus::deltaShots(int shots)
{
   m_ammo += shots;

   if (m_ammo > 100)
   {
      m_ammo = 100;
   }

   if (m_statusEventOriginator)
   {
      StatusChangedEvent event(GameStatusTypeEnum::shots, m_ammo, 100.0f);
      m_statusEventOriginator->dispatchEvent(&event);

      if (m_ammo <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::shots);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (m_ammo < 0)
   {
      m_ammo = 0;
   }
}

float GameStatus::getShield(void)
{
   return m_shield;
}

void GameStatus::deltaShield(float shield)
{
   if ((m_shield == 0) && (shield > 0))
   {
      m_shieldObj->restartShield();
   }

   m_shield += shield;

   if (m_shield > 100)
   {
      m_shield = 100;
   }

   if (m_statusEventOriginator)
   {
      StatusChangedEvent event(GameStatusTypeEnum::shield, m_shield, 100.0f);
      m_statusEventOriginator->dispatchEvent(&event);

      if (m_shield <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::shield);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (m_shield < 0.0f)
   {
      m_shield = 0.0f;
   }

}


void GameStatus::registerShieldObject(Shield* shieldObj)
{
   m_shieldObj = shieldObj;
}


float GameStatus::getFuel(void)
{
   return m_fuel;
}

void GameStatus::deltaFuel(float fuel)
{
   m_fuel += fuel;

   if (m_fuel > 100)
   {
      m_fuel = 100;
   }

   if (m_statusEventOriginator)
   {
      StatusChangedEvent event(GameStatusTypeEnum::fuel, m_fuel, 100.0f);
      m_statusEventOriginator->dispatchEvent(&event);

      if (m_fuel <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::fuel);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (m_fuel < 0.0f)
   {
      m_fuel = 0.0f;
   }
}

int GameStatus::getCredits(void)
{
   return m_credits;
}

void  GameStatus::deltaCredits(int credits)
{
   m_credits += credits;

   if (m_statusEventOriginator)
   {
      StatusChangedEvent event(GameStatusTypeEnum::credits, m_credits, 100.0f);
      m_statusEventOriginator->dispatchEvent(&event);

      if (m_credits <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::credits);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

float GameStatus::getDamage(void)
{
   return m_damage;
}

void GameStatus::deltaDamage(float deltaDamage)
{
   m_damage += deltaDamage;

   if (m_statusEventOriginator)
   {
      StatusChangedEvent event(GameStatusTypeEnum::damage, m_damage, 100.0f);
      m_statusEventOriginator->dispatchEvent(&event);

      if (m_damage <= 0.0f)
      {
         StatusResourceDepletedEvent event(GameStatusTypeEnum::damage);
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }

   if (m_damage < 0.0f)
   {
      m_damage = 0.0f;
   }

}

