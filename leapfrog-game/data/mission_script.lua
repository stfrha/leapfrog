-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.


-- Global variables
currentState = "state1"


-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function setupMissionStateScene()

   if currentState == "state1" then
      if currentScene == "landing_scene.xml" then
         -- Add objects from file mission-state-scene xml file (can it be incorporated into this script?)
            -- In this case, there is nothing extra to load
         -- Register all triggers required for this mission-state-scene
         c_registerPropertyTrigger("leapfrog1", 1, 3, "insideRange", 200, 400);
         
         -- Register all events required for this mission-state-scene
         c_registerEventHandler("leapfrog1", "OpTr", 1, 3)

      elseif currentScene == "deep_space_scene.xml" then

      elseif currentScene == "orbit_scene.xml" then

      end

   elseif currentState == "state2" then

      if currentScene == "landing_scene.xml" then
         c_addMissionStateSceneObjects("test_mission/state2/landing_scene.xml")
         c_registerPropertyTrigger("leapfrog1", 1, 3, "insideRange", 200, 400);

      elseif currentScene == "deep_space_scene.xml" then
      
      elseif currentScene == "orbit_scene.xml" then
      
      end
   end
end

function missionStateSceneEventHandler(eventId, actorName, parameter1)
   if currentState == "state1" then
      if currentScene == "landing_scene.xml" then
         if actorName == "leapfrog1" then
            if eventId == "OpTr" then
               
               if parameter1 == 3 then

                  -- Play dialog
                  currentState = "state2"
                  
                  -- Clear triggers and event handler
                  -- c_clearAllTriggersAndEvents()
                  
                  setupMissionStateScene()
               end
            end 
         end
      elseif currentScene == "deep_space_scene.xml" then
      elseif currentScene == "orbit_scene.xml" then
      end

   elseif currentState == "state2" then

      if currentScene == "landing_scene.xml" then
      elseif currentScene == "deep_space_scene.xml" then
      elseif currentScene == "orbit_scene.xml" then
      
      end
   end
end
