#include "FpsEstimator.h"

using namespace oxygine;

FpsEstimator::FpsEstimator(oxygine::Actor* parent) :
   m_state(FpsEstimatorStates::idle),
   m_count(0),
   m_startTime(0),
   m_time(0)
{
   attachTo(parent);
}

float FpsEstimator::getFps(void)
{
   if (m_state != FpsEstimatorStates::finished)
   {
      return -1.0f;
   }

   return (float)m_count / ((float)m_time - (float)m_startTime) * 1000.0f;

}

void FpsEstimator::doUpdate(const oxygine::UpdateState& us)
{
   if (m_state == FpsEstimatorStates::idle)
   {
      m_startTime = getTimeMS();
      m_count = 0;
      m_state = FpsEstimatorStates::measureing;
   }
   else if (m_state == FpsEstimatorStates::measureing)
   {
      m_time = getTimeMS();

      m_count++;

      if (m_count > 60)
      {
         m_count--;
         m_state = FpsEstimatorStates::finished;
      }
   }
}

 
