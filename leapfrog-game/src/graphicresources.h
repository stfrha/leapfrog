#pragma once

#include "oxygine-framework.h"
#include <map>
#include <string>

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

   // Per-scene resources: data/scene_resources.xml maps scene files to
   // resource files (e.g. landing_scene.xml -> res_mining.xml). The set is
   // loaded when a scene starts and freed when a scene with another set
   // starts, so big scenery textures only use memory where they're needed.
   std::map<std::string, std::string> m_sceneResMap;
   bool m_sceneResMapRead;
   std::string m_sceneResFile;      // resource file currently loaded as scene resources

   void readSceneResourceMap(void);

public:

   GraphicResources();

   void loadResource(ResourceTypeEnum type, const std::string& source);
   void unLoadResource(ResourceTypeEnum type);
   const oxygine::Resources& getResources(ResourceTypeEnum type);
   void freeResources(void);

   // Loads the resource set that scene_resources.xml lists for sceneFile,
   // freeing the previous scene's set. Does nothing if the set is the same.
   void loadSceneResources(const std::string& sceneFile);

   // Texture lookup for scene objects: the current scene's resources first,
   // then the game resources (res.xml).
   oxygine::ResAnim* getResAnim(const std::string& name);
};

extern GraphicResources g_GraphRes;
