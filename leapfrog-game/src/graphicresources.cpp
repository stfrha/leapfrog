#include "graphicresources.h"
#include <algorithm>

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

// Splits a resources attribute into file names (space, tab or comma
// separated). res.xml is always loaded as the game resources, so it's left
// out.
static std::vector<std::string> splitResourceList(const std::string& s)
{
   std::vector<std::string> files;
   std::string cur;

   for (size_t i = 0; i <= s.size(); i++)
   {
      char c = (i < s.size()) ? s[i] : ' ';
      if ((c == ' ') || (c == '\t') || (c == ',') || (c == ';') || (c == '\r') || (c == '\n'))
      {
         if (!cur.empty() && (cur != "res.xml") &&
            (std::find(files.begin(), files.end(), cur) == files.end()))
         {
            files.push_back(cur);
         }
         cur.clear();
      }
      else
      {
         cur += c;
      }
   }

   return files;
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
      m_sceneResMap[n.attribute("file").as_string()] = splitResourceList(n.attribute("resources").as_string());
   }
}

void GraphicResources::loadSceneResources(const std::string& sceneFile)
{
   if (!m_sceneResMapRead)
   {
      readSceneResourceMap();
   }

   std::vector<std::string> wanted;
   std::map<std::string, std::vector<std::string> >::const_iterator it = m_sceneResMap.find(sceneFile);
   if (it != m_sceneResMap.end())
   {
      wanted = it->second;
   }

   // Free the files the new scene doesn't use (first, so the memory is
   // free before new textures load)
   for (std::map<std::string, Resources*>::iterator r = m_sceneResources.begin(); r != m_sceneResources.end(); )
   {
      if (std::find(wanted.begin(), wanted.end(), r->first) == wanted.end())
      {
         logs::messageln("GraphicResources: freeing scene resources %s", r->first.c_str());
         r->second->free();
         delete r->second;
         r = m_sceneResources.erase(r);
      }
      else
      {
         ++r;
      }
   }

   // Load the ones not loaded yet
   m_sceneResFiles.clear();
   for (size_t i = 0; i < wanted.size(); i++)
   {
      const std::string& resFile = wanted[i];

      if (m_sceneResources.find(resFile) == m_sceneResources.end())
      {
         if (!file::exists(resFile))
         {
            logs::messageln("GraphicResources: %s (for %s) not found", resFile.c_str(), sceneFile.c_str());
            continue;
         }

         logs::messageln("GraphicResources: loading scene resources %s for %s", resFile.c_str(), sceneFile.c_str());
         Resources* res = new Resources();
         res->loadXML(resFile);
         m_sceneResources[resFile] = res;
      }

      m_sceneResFiles.push_back(resFile);
   }
}

void GraphicResources::freeSceneResources(void)
{
   for (std::map<std::string, Resources*>::iterator r = m_sceneResources.begin(); r != m_sceneResources.end(); ++r)
   {
      r->second->free();
      delete r->second;
   }
   m_sceneResources.clear();
   m_sceneResFiles.clear();
}

ResAnim* GraphicResources::getResAnim(const std::string& name)
{
   for (size_t i = 0; i < m_sceneResFiles.size(); i++)
   {
      std::map<std::string, Resources*>::const_iterator r = m_sceneResources.find(m_sceneResFiles[i]);
      if (r != m_sceneResources.end())
      {
         ResAnim* ra = r->second->getResAnim(name, ep_ignore_error);
         if (ra)
         {
            return ra;
         }
      }
   }

   return m_gameResource.getResAnim(name);
}

void GraphicResources::freeResources(void)
{
   m_hudResource.free();
   m_gameResource.free();
   m_sceneResource.free();
   freeSceneResources();
   m_leapfrogResource.free();
   m_missionResource.free();
   m_allegianceResource.free();
   m_enemyResource.free();
}


GraphicResources g_GraphRes;
