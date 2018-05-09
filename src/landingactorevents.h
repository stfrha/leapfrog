#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class LandingActorTranstToDeepSpaceEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'a', 'T', 'D') };
   LandingActorTranstToDeepSpaceEvent(void) :Event(EVENT) {}
};

