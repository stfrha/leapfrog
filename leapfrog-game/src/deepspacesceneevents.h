#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class DeepSpaceSceneTransitToOrbitEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('D', 's', 'T', 'O') };
   DeepSpaceSceneTransitToOrbitEvent(void) :Event(EVENT) {}
};

