#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class PickedUpEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('P', 'u', 'P', 'U') };
   PickedUpEvent(void) :Event(EVENT) {}
};

