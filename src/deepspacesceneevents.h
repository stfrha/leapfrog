#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class DeepSpaceSceneTranstToOrbitEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('D', 's', 'T', 'O') };
   DeepSpaceSceneTranstToOrbitEvent(void) :Event(EVENT) {}
};

