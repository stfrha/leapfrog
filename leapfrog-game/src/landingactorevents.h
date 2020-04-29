#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

// Parameter int is "leaving how":
// 1 = deep space
// Transition between ground-scenes are triggered by triggered events


class ExitLandingSceneEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'a', 'T', 'D') };
   int m_parameter;
   ExitLandingSceneEvent(int parameter) : Event(EVENT), m_parameter(parameter) {}
};

