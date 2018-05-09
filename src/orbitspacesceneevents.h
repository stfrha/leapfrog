#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class OrbitSpaceOrbitEstablished : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 's', 'O', 'E') };
   OrbitSpaceOrbitEstablished(void) :Event(EVENT) {}
};
