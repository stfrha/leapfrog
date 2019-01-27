#include "groupindexsource.h"

using namespace oxygine;

// Instansiate global class
GroupIndexSource g_GroupIndexSource;

GroupIndexSource::GroupIndexSource() :
   m_source(1)
{}

int GroupIndexSource::getNewGroupIndex(void)
{
   int i = m_source;
   m_source++;
   return i;
}

