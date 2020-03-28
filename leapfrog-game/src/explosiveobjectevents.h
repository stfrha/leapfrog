#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class ExplosiveObjectExplodedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('E', 'x', 'E', 'X') };
   ExplosiveObjectExplodedEvent(void) :Event(EVENT) {}
};


