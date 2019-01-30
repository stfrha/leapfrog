#pragma once

#include "oxygine-framework.h"

DECLARE_SMART(GameStatus, spGameStatus)

// Needs to be an actor to send events
class GameStatus : public oxygine::ref_counter
{
private:
   int m_ammo;
   float m_shield;
   float m_fuel;
   int m_credits;
   float m_damage;

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
   void deltaFuel(float fuel);

   int getCredits(void);
   void  deltaCredits(int credits);

   float getDamage(void);
   void deltaDamage(float damage);

};