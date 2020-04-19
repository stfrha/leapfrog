#include "reentrysceneactor.h"


using namespace oxygine;

ReentrySceneActor::ReentrySceneActor(Resources& gameResources) :
   SceneActor(gameResources, NULL, 0.4f)
{
   // I should probably load resources that are uniuqe here
   setPanorateMode(center);

   m_leapfrog->goToEnvironment(ENV_ORBIT);

}

