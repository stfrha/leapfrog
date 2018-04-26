#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class LaunchSiteLeapfrogLandedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 's', 'L', 'L') };
   LaunchSiteLeapfrogLandedEvent(void) :Event(EVENT) {}
};

