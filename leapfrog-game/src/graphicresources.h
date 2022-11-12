#pragma once

#include "oxygine-framework.h"

class GraphicResources 
{

public:
   enum ResourceTypeEnum
   {
      hud,
      game,
      scene,
      mission,
      leapfrog,
      allegiance,
      enemy
   };

private:
   oxygine::Resources m_hudResource;
   oxygine::Resources m_gameResource;
   oxygine::Resources m_sceneResource;
   oxygine::Resources m_missionResource;
   oxygine::Resources m_leapfrogResource;
   oxygine::Resources m_allegianceResource;
   oxygine::Resources m_enemyResource;

public:

   GraphicResources();

   void loadResource(ResourceTypeEnum type, const std::string& source);
   void unLoadResource(ResourceTypeEnum type);
   const oxygine::Resources& getResources(ResourceTypeEnum type);
   void freeResources(void);
};

extern GraphicResources g_GraphRes;
