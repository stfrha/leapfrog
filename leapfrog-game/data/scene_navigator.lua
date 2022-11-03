-- currentScene and gameProgress should be read from file to begin with
-- perhaps they are read from files and then sent as inputs to this script

currentScene = "New Game"
currentMission = 0

-- currentSceneType is: 0 = landing, 1 = deep space, 2 = orbit, 3 = hyperspace
currentSceneType = 0

gameProgress = 0
nextLfFile = "leapfrog_landing.xml"
nextLfPosX = 415
nextLfPosY = 657
nextLfConfiguration = 2

-- This is used to determine if the nextLfxxxxx variables declared above should
-- be used when the Leapfrog is created (in the setupMissionStateScene function)
-- or if the exit-entry pair of the lua_sceneExitHandler function below
-- is used.
-- The nextLfxxxxx variables here are only used (useInitialLfPos == false) when 
-- starting a new game or starting a saved game. 
useInitialLfPos = false

-- This function starts a new or saved game. If used for a new
-- game, the currentMission needs only be set to zero for this 
-- function to initialize the first mission and selects its scene.
-- If used for a saved game, the currentMission must be set to >0.
-- The other variables: sceneType, currentState and currentScene
-- must be set using the lua_forceCurrentScene before this is called.
function lua_startInitialScene()
   if currentMission == 0 then
      currentScene = "landing_scene.xml"
      sceneType = 0
      currentMission = 1
      currentState = 1
      useInitialLfPos = false -- For initial start, the LF file, pos and prop data is set by the setupMissionStateScene    
   end

   c_startScene(currentScene, sceneType)
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

function lua_forceCurrentScene(newCurrentScene, newMission, newState, newSceneType)
   currentScene = newCurrentScene;
   currentMission = newMission;
   currentState = newState;
   currentSceneType = newSceneType
   
end
