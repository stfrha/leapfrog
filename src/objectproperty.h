#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"

class ObjectPropertyTriggeredEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 'p', 'T', 'r') };
   int m_triggedEventId;
   int m_propertyId;
   float m_propertyValue;
   ObjectPropertyTriggeredEvent(int eventId, int propertyId, float value) :
      Event(EVENT),
      m_triggedEventId(eventId),
      m_propertyId(propertyId),
      m_propertyValue(value)
   {}
};

class PropertyEventTrigger
{
public:
   enum TriggerType
   {
      ignore,
      insideRange,
      outsideRange
   };

private:
   int m_eventId;
   TriggerType	m_triggerType;	// outside range trigger, inside range trigger
   float m_upperLimit;
   float m_lowerLimit;

public:
   PropertyEventTrigger();
   PropertyEventTrigger(int eventId, TriggerType triggerType, float upperLimit, float lowerLimit);

   bool evaluateTrigger(float value);
   int getEventId(void);

};

class DualPropEventTrigger
{
private:
   int m_eventId;
   int m_prop1Id;
   int m_prop2Id;
   PropertyEventTrigger m_prop1Trigger;
   PropertyEventTrigger m_prop2Trigger;

public:
   DualPropEventTrigger();
   DualPropEventTrigger(int eventId, int prop1Id, int prop2Id, PropertyEventTrigger& prop1Trigger, PropertyEventTrigger& prop2Trigger);

   void updateProperty(int propId);
};

class CompoundObject;

class ObjectProperty
{
   typedef void(*ExtSetProperyCallback)();

private:
   bool m_extReadOnly;
   int m_id;
   float m_value;
   CompoundObject* m_object;
   oxygine::Event* m_extSetPropEvent;

   std::vector<PropertyEventTrigger> m_eventTriggers;
   std::vector<DualPropEventTrigger*> m_dualEventTriggers;

public:
   ObjectProperty();
   ObjectProperty(
      CompoundObject* obj,
      oxygine::Event* extSetPropEvent,
      int id,
      float value,
      bool readOnly = false);
   ~ObjectProperty();

   void setProperty(float value);
   void extSetProperty(float value);
   float getProperty(void);
   void registerPropertyEventTrigger(
      int eventId,
      PropertyEventTrigger::TriggerType triggerType,
      float lower,
      float upper);
   void unregisterPropertyEventTrigger(int eventId);
   void registerDualPropEventTrigger(DualPropEventTrigger* trigger);
   void unregisterDualPropEventTrigger(DualPropEventTrigger* trigger);
};

