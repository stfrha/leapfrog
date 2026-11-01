#pragma once

#include "oxygine-framework.h"
#include <map>
#include <string>
#include <vector>

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

   // Per-scene resources: data/scene_resources.xml maps scene files to one
   // or more resource files, separated by spaces (e.g. deep_space_scene.xml
   // -> "res_space.xml res_mining.xml"). A scene's files are loaded when it
   // starts; files the next scene doesn't list are freed, files it shares
   // with the previous scene stay loaded. So big textures only use memory
   // where they're needed.
   std::map<std::string, std::vector<std::string> > m_sceneResMap;
   bool m_sceneResMapRead;

   // The current scene's resource files, in the order they are searched
   std::vector<std::string> m_sceneResFiles;
   std::map<std::string, oxygine::Resources*> m_sceneResources;   // loaded, by file name

   void readSceneResourceMap(void);
   void freeSceneResources(void);

public:

   GraphicResources();

   void loadResource(ResourceTypeEnum type, const std::string& source);
   void unLoadResource(ResourceTypeEnum type);
   const oxygine::Resources& getResources(ResourceTypeEnum type);
   void freeResources(void);

   // Loads the resource files that scene_resources.xml lists for sceneFile
   // and frees those of the previous scene that it doesn't list.
   void loadSceneResources(const std::string& sceneFile);

   // Texture lookup for scene objects and anything shown in a scene: the
   // current scene's resource files in their listed order, then the game
   // resources (res.xml).
   oxygine::ResAnim* getResAnim(const std::string& name);
};

extern GraphicResources g_GraphRes;
