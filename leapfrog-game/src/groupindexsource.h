#pragma once

#include "oxygine-framework.h"

// Needs to be an actor to send events
class GroupIndexSource
{
private:
   int m_source;

public:
   GroupIndexSource();

   int getNewGroupIndex();

};

extern GroupIndexSource g_GroupIndexSource;