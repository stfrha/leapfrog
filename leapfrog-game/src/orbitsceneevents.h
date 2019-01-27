#pragma once

#include "oxygine-framework.h"
#include "planetactor.h"

// Events that is fired for special conditions

class OrbitSceneLandingComplete : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 'S', 'L', 'C') };
   LandingResult m_results;
   OrbitSceneLandingComplete(LandingResult results) :
      Event(EVENT),
      m_results(results)
   {  }
};
