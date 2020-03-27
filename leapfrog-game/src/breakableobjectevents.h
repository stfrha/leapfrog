#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class BreakableObjectBreakEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('B', 'r', 'B', 'R') };
   BreakableObjectBreakEvent(void) :Event(EVENT) {}
};

class BreakableObjectNewObjectSpawnedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('B', 'r', 'S', 'W') };
   BreakableObjectNewObjectSpawnedEvent(void) :Event(EVENT) {}
};
