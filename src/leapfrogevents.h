#pragma once

#include "oxygine-framework.h"

class LeapfrogModeReachedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'f', 'M', 'r') };
   std::string extraInfo;
   LeapfrogModeReachedEvent(std::string id) :Event(EVENT), extraInfo(id) {}
};

