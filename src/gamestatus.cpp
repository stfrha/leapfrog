#include "gamestatus.h"
#include "gamestatusevents.h"

using namespace oxygine;

// Instansiate global class
GameStatus g_GameStatus;

GameStatus::GameStatus()
{
}

void GameStatus::initGameStatus(Actor* parent)
{
   m_parent = parent;
   // This should probably be read from a file or 
   // some non-volatile memory. But how?!?!
   m_leapFrogShots = 100;
   m_leapFrogShield = 0.5;
   m_credits = 248;
}

int GameStatus::getShots(void)
{
   return m_leapFrogShots;
}

void  GameStatus::deltaShots(int shots)
{
   m_leapFrogShots += shots;

   StatusChangedEvent event(StatusChangedEvent::GameStatusTypeEnum::shots, m_leapFrogShots, 100.0f);
   m_parent->dispatchEvent(&event);
}

float GameStatus::getShield(void)
{
   return m_leapFrogShield;
}

void GameStatus::deltaShield(float shield)
{
   m_leapFrogShield += shield;

   StatusChangedEvent event(StatusChangedEvent::GameStatusTypeEnum::shield, m_leapFrogShield, 1.0f);
   m_parent->dispatchEvent(&event);
}

float GameStatus::getFuel(void)
{
   return m_leapFrogFuel;
}

void GameStatus::deltaFuel(float fuel)
{
   m_leapFrogFuel += fuel;

   StatusChangedEvent event(StatusChangedEvent::GameStatusTypeEnum::fuel, m_leapFrogFuel, 100.0f);
   m_parent->dispatchEvent(&event);
}

int GameStatus::getCredits(void)
{
   return m_credits;
}

void  GameStatus::deltaCredits(int credits)
{
   m_credits += credits;

   StatusChangedEvent event(StatusChangedEvent::GameStatusTypeEnum::credits, m_credits, 100.0f);
   m_parent->dispatchEvent(&event);
}

