#pragma once

#include "oxygine-framework.h"

// Events that is fired when a property is set externally

class LeapfrogExtSetModeEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'x', 's', 'M') };
   LeapfrogExtSetModeEvent(void) :Event(EVENT) {}
};

class LeapfrogExtSetEnvEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'x', 's', 'E') };
   LeapfrogExtSetEnvEvent(void) :Event(EVENT) {}
};

class LeapfrogExtSetLegHealthEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'x', 's', 'L') };
   LeapfrogExtSetLegHealthEvent(void) :Event(EVENT) {}
};

// Events that is fired for special conditions

class LeapfrogModeReachedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'f', 'M', 'r') };
   std::string extraInfo;
   LeapfrogModeReachedEvent(std::string id) :Event(EVENT), extraInfo(id) {}
};

// A Stripper took something from the Leapfrog. m_loot is the Leapfrog
// property id of what it took (5 ammo, 6 shield, 7 fuel, 8 credits), 0 if
// it found nothing to take; Lua gets it as the event parameter.
class LeapfrogStrippedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'f', 'S', 't') };
   int m_loot;
   float m_amount;
   LeapfrogStrippedEvent(int loot, float amount) :Event(EVENT), m_loot(loot), m_amount(amount) {}
};
