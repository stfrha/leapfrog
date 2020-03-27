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
         -- Register event for landing on landingPad1
         c_registerEventHandler("landing_pad1", "LpLL", 0, 0)

         -- Play dialog
         c_addDialogMessage("Hello. Try to land on the landingpad, to your left.", "DEBUG", true, 0, 0)
      end

   elseif currentState == "state2" then
      if currentScene == "landing_scene.xml" then
         c_addMissionStateSceneObjects("landing_scene_state2.xml")
         c_registerEventHandler("landing_pad1", "LpTO", 0, 0)
      end

   elseif currentState == "state3" then
      if currentScene == "landing_scene.xml" then
         c_registerEventHandler("springHolder", "BrBR", 0, 0)
      end

   elseif currentState == "state4" then
      if currentScene == "landing_scene.xml" then
         c_registerEventHandler("bomb1", "ExEX", 0, 0)
      end


   end
end

function missionStateSceneEventHandler(eventId, actorName, parameter1)
   if currentState == "state1" then
      if currentScene == "landing_scene.xml" then
         if actorName == "landing_pad1" then
            if eventId == "LpLL" then
               
               -- Play dialog
               c_addDialogMessage("Welcome to the landing pad. Let me fill you up. A new obstacle has arrived in the grotto. Check it out.", "DEBUG", true, 0, 0)

               currentState = "state2"
               
               -- Clear triggers and event handler
               c_clearAllTriggersAndEvents()
               
               setupMissionStateScene()
            end 
         end
      end

   elseif currentState == "state2" then
      if currentScene == "landing_scene.xml" then
         if actorName == "landing_pad1" then
            if eventId == "LpTO" then
               
               -- Play dialog
               c_addDialogMessage("That's right. Git yourself over there.", "DEBUG", true, 0, 0)

               currentState = "state3"
               
               -- Clear triggers and event handler
               c_clearAllTriggersAndEvents()
               
               setupMissionStateScene()
            end 
         end
      end

   elseif currentState == "state3" then
      if currentScene == "landing_scene.xml" then
         if actorName == "springHolder" then
            if eventId == "BrBR" then
               
               -- Play dialog
               c_addDialogMessage("Good job, the spring is free. Now destroy the bomb.", "DEBUG", true, 0, 0)

               currentState = "state4"
               
               -- Clear triggers and event handler
               c_clearAllTriggersAndEvents()
               
               setupMissionStateScene()
            end 
         end
      end

   elseif currentState == "state4" then
      if currentScene == "landing_scene.xml" then
         if actorName == "bomb1" then
            if eventId == "ExEX" then
               
               -- Play dialog
               c_addDialogMessage("Mission Complete!!!", "DEBUG", true, 0, 0)

               currentState = "state5"
               
               -- Clear triggers and event handler
               c_clearAllTriggersAndEvents()
               
               setupMissionStateScene()
            end 
         end
      end
   end
end
