#pragma once

#include "oxygine-framework.h"
#include "planetactor.h"

// Events that is fired for special conditions

// Parameter int is "index of landing sites"
// -1 = Stranded, landed but missed all sites
// -2 = Burned up during re-entry
// -3 = Bounced back to deep space

class ExitOrbitSceneEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 'S', 'L', 'C') };
   int m_parameter;
   ExitOrbitSceneEvent(int parameter) : Event(EVENT), m_parameter(parameter) {}
};
