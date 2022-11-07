-- currentScene and gameProgress should be read from file to begin with
-- perhaps they are read from files and then sent as inputs to this script

currentScene = "New Game"
currentMission = 0

-- currentSceneType is: 0 = landing, 1 = deep space, 2 = orbit, 3 = hyperspace
currentSceneType = 0

gameProgress = 0

-- These variables are used in the mission_script to load the leapfrog at the 
-- correct position and configuration for the exit/entry combination. Thus, 
-- they are set here in the lua_sceneExitHandler function and then used in 
-- lua_setupMissionStateScene function. To use them, the useInitialConfiguration
-- must be set to false.
nextLfFile = "leapfrog_landing.xml"
nextLfPosX = 415
nextLfPosY = 657
nextLfMode = 2  -- 0 = Reset, 1 = deep space, 2 = Landing, 3 = Orbit, 4 == Reentry (What is the differnce between orbit and reentry?)
-- Or what is the difference between Deep space and orbit?

-- This is used to determine if the nextLfxxxxx variables declared above should
-- be used when the Leapfrog is created in the setupMissionStateScene function (false)
-- or if the exit-entry pair of the lua_sceneExitHandler function below
-- is used (true).
useInitialLfPos = false

lfDefined = false

-- This function starts a new or saved game. If used for a new
-- game, the currentMission needs only be set to zero for this 
-- function to initialize the first mission and selects its scene.
-- If used for a saved game, the currentMission must be set to >0.
-- The other variables: currentSceneType, currentState and currentScene
-- must be set using the lua_forceCurrentScene before this is called.
function lua_startInitialScene()
   if currentMission == 0 then
      currentScene = "landing_scene.xml"
      currentSceneType = 0
      currentMission = 1
      currentState = 1
      lfDefined = false
   end

   useInitialLfPos = true -- For initial start, the LF file, pos and prop data is set by the setupMissionStateScene    
   c_startScene(currentScene, currentSceneType)
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
            currentSceneType = 1

            nextLfFile = "leapfrog_deep_space.xml"
            nextLfPosX = 40
            nextLfPosY = 760
            nextLfMode = 1
            useInitialLfPos = false
            lfDefined = false

            c_saveGameStatus("deep_space_scene.xml", currentMission, currentState, currentSceneType)
         end
      end
   
   elseif exitSceneType == 1 then
      -- We are leaving deep space scene
      -- Determine next scene
      if currentScene == "deep_space_scene.xml" then
         if exitParameter == 3 then
            currentScene = "orbit_scene.xml"
            currentSceneType = 2
            
            nextLfFile = "leapfrog_deep_space.xml"
            nextLfPosX = -8
            nextLfPosY = 18
            nextLfMode = 3
            useInitialLfPos = false
            lfDefined = false
            
            c_saveGameStatus("orbit_scene.xml", currentMission, currentState, currentSceneType)
         end
      end
   
   elseif exitSceneType == 2 then
      -- We are leaving orbit scene
      -- Determine next scene
      if currentScene == "orbit_scene.xml" then
         currentScene = "landing_scene.xml"
         currentSceneType = 0  
         
         nextLfFile = "leapfrog_reentry.xml"
         nextLfPosX = 520
         nextLfPosY = 50
         nextLfMode = 2
         useInitialLfPos = false
         lfDefined = false
         
         c_saveGameStatus("landing_scene.xml", currentMission, currentState, currentSceneType)
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
