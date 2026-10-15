#include "graphicresources.h"

using namespace oxygine;



GraphicResources::GraphicResources() :
   m_sceneResMapRead(false)
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

void GraphicResources::readSceneResourceMap(void)
{
   m_sceneResMapRead = true;
   m_sceneResMap.clear();

   const std::string mapFile = "scene_resources.xml";

   if (!file::exists(mapFile))
   {
      logs::messageln("GraphicResources: no %s, no per-scene resources", mapFile.c_str());
      return;
   }

   file::buffer bf;
   file::read(mapFile.c_str(), bf);

   if (bf.size() == 0)
   {
      return;
   }

   pugi::xml_document doc;
   if (!doc.load_buffer(&bf.data[0], bf.size()))
   {
      logs::messageln("GraphicResources: could not parse %s", mapFile.c_str());
      return;
   }

   for (pugi::xml_node n = doc.child("sceneResources").child("scene"); n; n = n.next_sibling("scene"))
   {
      m_sceneResMap[n.attribute("file").as_string()] = n.attribute("resources").as_string();
   }
}

void GraphicResources::loadSceneResources(const std::string& sceneFile)
{
   if (!m_sceneResMapRead)
   {
      readSceneResourceMap();
   }

   std::string resFile;
   std::map<std::string, std::string>::const_iterator it = m_sceneResMap.find(sceneFile);
   if (it != m_sceneResMap.end())
   {
      resFile = it->second;
   }

   if (resFile == m_sceneResFile)
   {
      // Same set as the previous scene (e.g. another area of the same world)
      return;
   }

   if (!m_sceneResFile.empty())
   {
      logs::messageln("GraphicResources: freeing scene resources %s", m_sceneResFile.c_str());
      m_sceneResource.free();
      m_sceneResFile = "";
   }

   if (!resFile.empty())
   {
      if (file::exists(resFile))
      {
         logs::messageln("GraphicResources: loading scene resources %s for %s", resFile.c_str(), sceneFile.c_str());
         m_sceneResource.loadXML(resFile);
         m_sceneResFile = resFile;
      }
      else
      {
         logs::messageln("GraphicResources: %s (for %s) not found", resFile.c_str(), sceneFile.c_str());
      }
   }
}

ResAnim* GraphicResources::getResAnim(const std::string& name)
{
   if (!m_sceneResFile.empty())
   {
      ResAnim* ra = m_sceneResource.getResAnim(name, ep_ignore_error);
      if (ra)
      {
         return ra;
      }
   }

   return m_gameResource.getResAnim(name);
}

void GraphicResources::freeResources(void)
{
   m_hudResource.free();
   m_gameResource.free();
   m_sceneResource.free();
   m_sceneResFile = "";
   m_leapfrogResource.free();
   m_missionResource.free();
   m_allegianceResource.free();
   m_enemyResource.free();
}


GraphicResources g_GraphRes;
