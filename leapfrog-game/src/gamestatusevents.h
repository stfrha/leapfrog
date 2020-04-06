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

class ObjectProperty;

//class StatusChangedEvent : public oxygine::Event
//{
//public:
//   enum { EVENT = eventID('G', 's', 'S', 'C') };
//
//   GameStatusTypeEnum m_type;
//   ObjectProperty* m_property;
//   StatusChangedEvent(
//      GameStatusTypeEnum type,
//      ObjectProperty* property) :
//      Event(EVENT), 
//      m_type(type),
//      m_property(property)
//   {}
//};
//
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

class TransactResourceCommandEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('G', 's', 'T', 'R') };

   GameStatusTypeEnum m_type;
   float m_value;
   TransactResourceCommandEvent(
      GameStatusTypeEnum type,
      float value) :
      Event(EVENT),
      m_type(type),
      m_value(value)
   {}
};


