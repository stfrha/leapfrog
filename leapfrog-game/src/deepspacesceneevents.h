#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

// Parameter int is "leaving how":
// 1 = upper right
// 2 = upper left
// 3 = lower right
// 4 = lower left

class ExitDeepSpaceSceneEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('D', 's', 'T', 'O') };
   int m_parameter;
   ExitDeepSpaceSceneEvent(int parameter) : Event(EVENT), m_parameter(parameter) {}
};

