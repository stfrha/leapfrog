#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class LandingPadLeapfrogLandedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'p', 'L', 'L') };
   LandingPadLeapfrogLandedEvent(void) :Event(EVENT) {}
};

class LandingPadLeapfrogTakeOffEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'p', 'T', 'O') };
   LandingPadLeapfrogTakeOffEvent(void) :Event(EVENT) {}
};
