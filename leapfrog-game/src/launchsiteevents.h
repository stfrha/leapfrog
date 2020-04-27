#pragma once

#include "oxygine-framework.h"

class LaunchSite;

// Events that is fired for special conditions

class LaunchSiteLeapfrogLandedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 's', 'L', 'L') };
   LaunchSite* m_launchSite;
   LaunchSiteLeapfrogLandedEvent(LaunchSite* me) : Event(EVENT), m_launchSite(me) {}
};

class LaunchSequenceCompleteEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('L', 's', 'S', 'C') };
   LaunchSequenceCompleteEvent(void) :Event(EVENT) {}
};

