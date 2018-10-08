#include "gamestatus.h"
#include "gamestatusevents.h"

using namespace oxygine;

// Instansiate global class
GameStatus g_GameStatus;

GameStatus::GameStatus()
{
   // This should probably be read from a file or 
   // some non-volatile memory. But how?!?!
   m_leapFrogShots = 100;
   m_leapFrogFuel = 100.0f;
//   m_leapFrogShield = 100.0f;
   m_leapFrogShield = 0.0f;
   m_leapFrogDamage = 100.0f;
   m_credits = 248;
   m_statusEventOriginator = NULL;
}

void GameStatus::initGameStatus(Actor* statusEventOriginator)
{
   m_statusEventOriginator = statusEventOriginator;
}

int GameStatus::getShots(void)
{
   return m_leapFrogShots;
}

void  GameStatus::deltaShots(int shots)
{
   m_leapFrogShots += shots;

   StatusChangedEvent event(GameStatusTypeEnum::shots, m_leapFrogShots, 100.0f);
   
   if (m_statusEventOriginator)
   {
      m_statusEventOriginator->dispatchEvent(&event);
   }

   if (m_leapFrogShots <= 0.0f)
   {
      StatusResourceDepletedEvent event(GameStatusTypeEnum::shots);

      if (m_statusEventOriginator)
      {
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

float GameStatus::getShield(void)
{
   return m_leapFrogShield;
}

void GameStatus::deltaShield(float shield)
{
   m_leapFrogShield += shield;

   StatusChangedEvent event(GameStatusTypeEnum::shield, m_leapFrogShield, 100.0f);

   if (m_statusEventOriginator)
   {
      m_statusEventOriginator->dispatchEvent(&event);
   }

   if (m_leapFrogShield <= 0.0f)
   {
      StatusResourceDepletedEvent event(GameStatusTypeEnum::shield);

      if (m_statusEventOriginator)
      {
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

float GameStatus::getFuel(void)
{
   return m_leapFrogFuel;
}

void GameStatus::deltaFuel(float fuel)
{
   m_leapFrogFuel += fuel;

   StatusChangedEvent event(GameStatusTypeEnum::fuel, m_leapFrogFuel, 100.0f);

   if (m_statusEventOriginator)
   {
      m_statusEventOriginator->dispatchEvent(&event);
   }

   if (m_leapFrogFuel <= 0.0f)
   {
      StatusResourceDepletedEvent event(GameStatusTypeEnum::fuel);

      if (m_statusEventOriginator)
      {
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

int GameStatus::getCredits(void)
{
   return m_credits;
}

void  GameStatus::deltaCredits(int credits)
{
   m_credits += credits;

   StatusChangedEvent event(GameStatusTypeEnum::credits, m_credits, 100.0f);

   if (m_statusEventOriginator)
   {
      m_statusEventOriginator->dispatchEvent(&event);
   }

   if (m_credits <= 0.0f)
   {
      StatusResourceDepletedEvent event(GameStatusTypeEnum::credits);

      if (m_statusEventOriginator)
      {
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

float GameStatus::getDamage(void)
{
   return m_leapFrogDamage;
}

void GameStatus::deltaDamage(float Damage)
{
   m_leapFrogDamage += Damage;

   StatusChangedEvent event(GameStatusTypeEnum::damage, m_leapFrogDamage, 100.0f);

   if (m_statusEventOriginator)
   {
      m_statusEventOriginator->dispatchEvent(&event);
   }

   if (m_leapFrogDamage <= 0.0f)
   {
      StatusResourceDepletedEvent event(GameStatusTypeEnum::damage);

      if (m_statusEventOriginator)
      {
         m_statusEventOriginator->dispatchEvent(&event);
      }
   }
}

