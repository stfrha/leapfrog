#pragma once

#include "oxygine-framework.h"

class SetPanningSettingsEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('S', 'a', 'P', 'A') };

   CompoundObject* m_panningObject;
   SetPanningSettingsEvent(CompoundObject* panningObject) :
      Event(EVENT),
      m_panningObject(panningObject)
   {}
};
