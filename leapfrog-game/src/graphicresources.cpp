#include "graphicresources.h"

using namespace oxygine;



GraphicResources::GraphicResources()
{
}

void GraphicResources::loadResource(ResourceTypeEnum type, const std::string& source)
{
   switch(type)
   {
   case ResourceTypeEnum::hud:
      m_hudResource.loadXML(source);
      return;
   case ResourceTypeEnum::game:
      m_gameResource.loadXML(source);
      return;
   case ResourceTypeEnum::scene:
      m_sceneResource.loadXML(source);
      return;
   case ResourceTypeEnum::leapfrog:
      m_leapfrogResource.loadXML(source);
      return;
   case ResourceTypeEnum::mission:
      m_missionResource.loadXML(source);
      return;
   case ResourceTypeEnum::allegiance :
      m_allegianceResource.loadXML(source);
      return;
   case ResourceTypeEnum::enemy:
      m_enemyResource.loadXML(source);
      return;
   }
}

void GraphicResources::unLoadResource(ResourceTypeEnum type)
{
   switch (type)
   {
   case ResourceTypeEnum::hud:
      m_hudResource.unload();
      return;
   case ResourceTypeEnum::game:
      m_gameResource.unload();
      return;
   case ResourceTypeEnum::scene:
      m_sceneResource.unload();
      return;
   case ResourceTypeEnum::leapfrog:
      m_leapfrogResource.unload();
      return;
   case ResourceTypeEnum::mission:
      m_missionResource.unload();
      return;
   case ResourceTypeEnum::allegiance:
      m_allegianceResource.unload();
      return;
   case ResourceTypeEnum::enemy:
      m_enemyResource.unload();
      return;
   }
}

const oxygine::Resources& GraphicResources::getResources(ResourceTypeEnum type)
{
   switch (type)
   {
   case ResourceTypeEnum::hud:
      return m_hudResource;
   case ResourceTypeEnum::game:
      return m_gameResource;
   case ResourceTypeEnum::scene:
      return m_sceneResource;
   case ResourceTypeEnum::leapfrog:
      return m_leapfrogResource;
   case ResourceTypeEnum::mission:
      return m_missionResource;
   case ResourceTypeEnum::allegiance:
      return m_allegianceResource;
   case ResourceTypeEnum::enemy:
      return m_enemyResource;
   default:
      return m_gameResource;
   }
}


GraphicResources g_GraphRes;
