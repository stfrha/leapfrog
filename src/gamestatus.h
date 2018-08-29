#pragma once

#include "oxygine-framework.h"

// Needs to be an actor to send events
class GameStatus
{
private:
   int m_leapFrogShots;
   float m_leapFrogShield;
   float m_leapFrogFuel;
   int m_credits;

   // originators are used to dispatch events (since this global object is bad for being an actor)
   // and because (I presume) event goes down and back up a hiearchy branch which may differ
   // for different status views
   oxygine::Actor* m_statusEventOriginator;

public:
	GameStatus();

   void initGameStatus(oxygine::Actor* originator);

   int getShots(void);
   void  deltaShots(int shots);

   float getShield(void);
   void deltaShield(float shield);

   float getFuel(void);
   void deltaFuel(float Fuel);

   int getCredits(void);
   void  deltaCredits(int Credits);

};

extern GameStatus g_GameStatus;