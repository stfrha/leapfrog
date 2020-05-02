#pragma once

#include "oxygine-framework.h"

enum ObjectResourcesTypeEnum
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
//   ObjectResourcesTypeEnum m_type;
//   ObjectProperty* m_property;
//   StatusChangedEvent(
//      ObjectResourcesTypeEnum type,
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

   ObjectResourcesTypeEnum m_type;
   StatusResourceDepletedEvent(
      ObjectResourcesTypeEnum type) :
      Event(EVENT),
      m_type(type)
   {}
};

class TransactResourceCommandEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('G', 's', 'T', 'R') };

   ObjectResourcesTypeEnum m_type;
   float m_value;
   TransactResourceCommandEvent(
      ObjectResourcesTypeEnum type,
      float value) :
      Event(EVENT),
      m_type(type),
      m_value(value)
   {}
};


