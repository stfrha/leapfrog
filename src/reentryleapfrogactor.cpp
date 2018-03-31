#include "ReentryCompositeActor.h"


using namespace oxygine;

ReentryCompositeActor::ReentryCompositeActor(Resources& gameResources)
{
   //// I should probably load resources that are uniuqe here
   //setPanorateMode(PME_FIX);

   //createLeapFrog(gameResources);

   //m_leapfrog->goToMode(LFM_ORBIT);
   //m_leapfrog->goToEnvironment(ENV_ORBIT);

   m_planetActor = new PlanetActor(gameResources);
   m_planetActor->setAnchor(0.5f, 0.5f);
   m_planetActor->setPosition(-30.0f, 20.0f);
//   m_planetActor->setScale(0.075f);
   m_planetActor->setScale(0.3f);
   m_planetActor->attachTo(this);

}

