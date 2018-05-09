#include "ReentrySceneActor.h"


using namespace oxygine;

ReentrySceneActor::ReentrySceneActor(Resources& gameResources) :
   SceneActor(gameResources, 0.4f)
{
   // I should probably load resources that are uniuqe here
   setPanorateMode(PME_CENTER);

   createLeapFrog(gameResources);

   m_leapfrog->goToEnvironment(ENV_ORBIT);

}

