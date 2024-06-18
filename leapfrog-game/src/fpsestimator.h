#pragma once

#include "oxygine-framework.h"

DECLARE_SMART(FpsEstimator, spFpsEstimator);

class FpsEstimator : public oxygine::Actor
{

   enum FpsEstimatorStates
   {
      idle,
      measureing,
      finished
   };

private:
   FpsEstimatorStates m_state;
   int m_count;
   oxygine::timeMS m_startTime;
   oxygine::timeMS m_time;


public:
   FpsEstimator(oxygine::Actor* parent);

   // I need at least one virtual member to make the 
   // the class work with dynamic_cast
   virtual ~FpsEstimator()
   {}


   float getFps(void);


protected:
	void doUpdate(const oxygine::UpdateState& us) override;
};
