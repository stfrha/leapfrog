#pragma once

#include "oxygine-framework.h"

// Events that is fired when a property is set externally

class LeapfrogExtSetModeEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'x', 's', 'M') };
   LeapfrogExtSetModeEvent(void) :Event(EVENT) {}
};

class LeapfrogExtSetEnvEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'x', 's', 'E') };
   LeapfrogExtSetEnvEvent(void) :Event(EVENT) {}
};

// Events that is fired for special conditions

class LeapfrogModeReachedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 'f', 'M', 'r') };
   std::string extraInfo;
   LeapfrogModeReachedEvent(std::string id) :Event(EVENT), extraInfo(id) {}
};

