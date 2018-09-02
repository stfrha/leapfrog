#pragma once

#include "oxygine-framework.h"

enum GameStatusTypeEnum
{
   fuel,
   credits,
   shots,
   shield,
   damage
};


// Events that are sent when status is changed

class StatusChangedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('G', 's', 'S', 'C') };

   GameStatusTypeEnum m_type;
   float m_value;
   float m_max;
   StatusChangedEvent(
      GameStatusTypeEnum type, 
      float value, 
      float max) : 
      Event(EVENT), 
      m_value(value),
      m_max(max),
      m_type(type)
   {}
};

class StatusResourceDepletedEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('G', 's', 'R', 'D') };

   GameStatusTypeEnum m_type;
   StatusResourceDepletedEvent(
      GameStatusTypeEnum type) :
      Event(EVENT),
      m_type(type)
   {}
};

