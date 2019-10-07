#pragma once

#include "oxygine-framework.h"

// I have yet not been able to send these events without
// crashing the program. It is about finding a common
// place to send them that is the problem. 

class CompoundObjectCreatedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('C', 'o', 'C', 'R') };
   CompoundObjectCreatedEvent(void) :Event(EVENT) {}
};

class CompoundObjectDeathEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('C', 'o', 'D', 'E') };
   CompoundObjectDeathEvent(void) :Event(EVENT) {}
};
