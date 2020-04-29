-- currentScene and gameProgress should be read from file to begin with
-- perhaps they are read from files and then sent as inputs to this script

currentScene = "New Game"
gameProgress = 0

function lua_startInitialScene()
   if currentScene == "New Game" then
      currentScene = "landing_scene.xml"
      sceneType = 0
      
      c_startScene(currentScene, sceneType)

   end
end


-- exitSceneType is: 0 = landing, 1 = deep space, 2 = orbit, 3 = hyperspace
-- exitParameter is additional information related to the exit scene type

-- For exiting landing scene:
-- Parameter int is "leaving how":
-- 1 = deep space
-- Transition between ground-scenes are triggered by triggered events

-- For exiting deep space scene:
-- Parameter int is "leaving how":
-- 1 = upper right
-- 2 = upper left
-- 3 = lower right
-- 4 = lower left

-- For exiting orbit scene
-- Parameter int is "index of landing sites"
-- -1 = Stranded, landed but missed all sites
-- -2 = Burned up during re-entry
-- -3 = Bounced back to deep space

-- Exiting hyperspace scene is TBD

function lua_sceneExitHandler(exitSceneType, exitParameter)
   local sceneType = 0

   if exitSceneType == 0 then
      -- We are leaving landing scene
      -- Determine next scene
      if currentScene == "landing_scene.xml" then
         if exitParameter == 1 then
            currentScene = "deep_space_scene.xml"
            sceneType = 1
         end
      end
   
   elseif exitSceneType == 1 then
      -- We are leaving deep space scene
      -- Determine next scene
      if currentScene == "deep_space_scene.xml" then
         if exitParameter == 3 then
            currentScene = "orbit_scene.xml"
            sceneType = 2
         end
      end
   
   elseif exitSceneType == 2 then
      -- We are leaving orbit scene
      -- Determine next scene
      if currentScene == "orbit_scene.xml" then
         currentScene = "landing_scene.xml"
         sceneType = 0  
      end
   end

   c_startScene(currentScene, sceneType)

end

function lua_forceCurrentScene(newCurrentScene)
   currentScene = newCurrentScene;
end
