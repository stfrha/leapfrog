#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class LandingActorTransitToDeepSpaceEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'a', 'T', 'D') };
   LandingActorTransitToDeepSpaceEvent(void) :Event(EVENT) {}
};

