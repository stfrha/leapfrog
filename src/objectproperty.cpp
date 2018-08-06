#include "objectproperty.h"
#include "compoundobject.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

PropertyEventTrigger::PropertyEventTrigger() :
   m_eventId(0),
   m_triggerType(ignore),
   m_upperLimit(0.0f),
   m_lowerLimit(0.0f)
{
}

PropertyEventTrigger::PropertyEventTrigger(int eventId, TriggerType triggerType, float upperLimit, float lowerLimit) :
   m_eventId(eventId),
   m_triggerType(triggerType),
   m_upperLimit(upperLimit),
   m_lowerLimit(lowerLimit)
{
}

bool PropertyEventTrigger::evaluateTrigger(float value)
{
   if (m_triggerType == insideRange)
   {
      return ((value >= m_lowerLimit) && (value <= m_upperLimit));
   }
   else if (m_triggerType == outsideRange)
   {
      return ((value < m_lowerLimit) || (value > m_upperLimit));
   }

   return false;
}

int PropertyEventTrigger::getEventId(void)
{
   return m_eventId;
}


ObjectProperty::ObjectProperty() :
   m_object(NULL),
   m_extSetPropEvent(NULL),
   m_extReadOnly(false),
   m_value(0.0f),
   m_id(0)
{
}

ObjectProperty::ObjectProperty(
   CompoundObject* obj,
   Event* extSetPropEvent,
   int id,
   float value,
   bool readOnly) :
   m_object(obj),
   m_extSetPropEvent(extSetPropEvent),
   m_extReadOnly(readOnly),
   m_value(value),
   m_id(id)
{
}

ObjectProperty::~ObjectProperty()
{
   m_eventTriggers.clear();
   m_dualEventTriggers.clear();
}

void ObjectProperty::setProperty(float value)
{
   m_value = value;

   // Handeling an even may cause it to be removed as a trigger from this 
   // list which would invalidate the iterator. Therefore we build a list of
   // all events to be sent and send them after the loop
   vector<ObjectPropertyTriggeredEvent> m_eventList;

   for (auto it = m_eventTriggers.begin(); it != m_eventTriggers.end(); ++it)
   {
      if (it->evaluateTrigger(value))
      {
         // Trigger event
         m_eventList.push_back(ObjectPropertyTriggeredEvent(it->getEventId(), m_id, value));
      }
   }

   for (auto it = m_eventList.begin(); it != m_eventList.end(); ++it)
   {
      m_object->dispatchEvent(&(*it));
   }

   for (auto it = m_dualEventTriggers.begin(); it != m_dualEventTriggers.end(); ++it)
   {
      (*it)->updateProperty(m_id);
   }
}

// This is used to set properties from outside of the object that 
// owns the property. 
void ObjectProperty::extSetProperty(float value)
{
   if (!m_extReadOnly)
   {
      setProperty(value);

      //test if event is set, and only dispatch event in that case
      if (m_extSetPropEvent)
      {
         m_object->dispatchEvent(m_extSetPropEvent);
      }
   }
}

float ObjectProperty::getProperty(void)
{
   return m_value;
}

void ObjectProperty::registerPropertyEventTrigger(
   int eventId,
   PropertyEventTrigger::TriggerType triggerType,
   float lower,
   float upper)
{
   m_eventTriggers.push_back(PropertyEventTrigger(eventId, triggerType, upper, lower));
}

void ObjectProperty::unregisterPropertyEventTrigger(int eventId)
{
   // iterate all triggers and remove those the has eventId
   for (auto it = m_eventTriggers.begin(); it != m_eventTriggers.end();)
   {
      if (it->getEventId() == eventId)
      {
         it = m_eventTriggers.erase(it);
      }
      else
      {
         ++it;
      }
   }
}


void ObjectProperty::registerDualPropEventTrigger(DualPropEventTrigger* trigger)
{
   m_dualEventTriggers.push_back(trigger);
}

void ObjectProperty::unregisterDualPropEventTrigger(DualPropEventTrigger* trigger)
{
   m_dualEventTriggers.erase(
      remove(
         m_dualEventTriggers.begin(),
         m_dualEventTriggers.end(), trigger),
      m_dualEventTriggers.end());
}

DualPropEventTrigger::DualPropEventTrigger() :
   m_eventId(0),
   m_prop1Id(0),
   m_prop2Id(0),
   m_prop1Trigger(0, PropertyEventTrigger::ignore, 0.0f, 0.0f),
   m_prop2Trigger(0, PropertyEventTrigger::ignore, 0.0f, 0.0f)
{
}

DualPropEventTrigger::DualPropEventTrigger(
   int eventId,
   int prop1Id,
   int prop2Id,
   PropertyEventTrigger& prop1Trigger,
   PropertyEventTrigger& prop2Trigger) :
   m_eventId(eventId),
   m_prop1Id(prop1Id),
   m_prop2Id(prop2Id),
   m_prop1Trigger(prop1Trigger),
   m_prop2Trigger(prop2Trigger)
{

}

void DualPropEventTrigger::updateProperty(int propId)
{
   //if (propId == m_prop1Id)
   //{
   //	if (m_prop1Trigger.evaluateTrigger(m_))
   //	{
   //		// use pointer to CompoundObject to read value of the 
   //		// prop2Id into p2Val
   //		float p2Val /* = compoundObject->getValue(m_prop2Id)*/;
   //		if (m_prop2Trigger.evaluateTrigger(p2Val))
   //		{
   //			// Trigger m_eventId

   //		}
   //	}
   //}
   //else if (propId == m_prop2Id)
   //{
   //	if (m_prop2Trigger.evaluateTrigger(propId))
   //	{
   //		// use pointer to CompoundObject to read value of the 
   //		// prop1Id into p1Val
   //		float p1Val /* = compoundObject->getValue(m_prop1Id)*/;
   //		if (m_prop1Trigger.evaluateTrigger(p1Val))
   //		{
   //           // Trigger m_eventId
   //		}
   //	}
   //}
}
