#pragma once

#include "oxygine-framework.h"

// Events that is fired for special conditions

class ObjectProperty;

class ObjectPropertyChangedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 'p', 'P', 'C') };
   int m_propId;
   ObjectProperty* m_property;

   ObjectPropertyChangedEvent(int propId, ObjectProperty* property) :
      Event(EVENT),
      m_propId(propId),
      m_property(property)
   {}
};

