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

class SceneTimeoutEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('S', 'c', 'T', 'O') };
   int m_timerId;

   SceneTimeoutEvent(int id) :
      Event(EVENT),
      m_timerId(id)
   {}
};
