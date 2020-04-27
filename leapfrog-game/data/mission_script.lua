-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.


-- Global variables
currentState = "state1"


-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function setupMissionStateScene()

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()

   if currentState == "state1" then
      if currentScene == "landing_scene.xml" then

         -- Start timer, will send LuTO event when timed out 
         state1Timer = c_startSceneTimer(240)
         
         -- Register event for landing on landingPad1
         c_registerEventHandler("landing_pad1", "LpLL", 0, 0)
         
         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 1)

         -- Play dialog
         c_addDialogMessage("Hello. Try to land on the landing pad, to your left.", "DEBUG", true, 0, 0)
      end

   elseif currentState == "state2" then
      if currentScene == "landing_scene.xml" then
         c_addMissionStateSceneObjects("landing_scene_state2.xml")
         
         c_registerEventHandler("landing_pad1", "LpTO", 0, 0)
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 0)

         -- Play dialog
         c_addDialogMessage("Try not to explode the bomd, just yet.", "DEBUG", true, 0, 0)
         -- Set bomb to be very in-sensitivity
         c_setObjectProperty("bomb1", 0, 250000)

      end

   elseif currentState == "state3" then
      if currentScene == "landing_scene.xml" then
         c_registerEventHandler("coDestroyable", "BrBR", 0, 0)
         c_registerEventHandler("bomb1", "ExEX", 0, 0)
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
         if eventId == "ScTO" and parameter1 == state1Timer then

            -- Play dialog
            c_addDialogMessage("The landing pad is the filled square on your map. Get there!", "DEBUG", true, 0, 0)
            c_addPositionedChildObject("leapfrog_reentry.xml", "leapfrog1", 537, 481)
            c_addPositionedChildObject("medium_asteroid.xml", "dummy1", 500, 500)

         end
         if actorName == "landing_pad1" and eventId == "LpLL" then
            
            -- Play dialog
            c_addDialogMessage("Welcome to the landing pad. Let me fill you up. A new obstacle has arrived in the grotto. Check it out.", "DEBUG", true, 0, 0)

            currentState = "state2"
            
            setupMissionStateScene()
         end
      end

   elseif currentState == "state2" then
      if currentScene == "landing_scene.xml" then
         if actorName == "landing_pad1" and eventId == "LpTO" then
               
            -- Play dialog
            c_addDialogMessage("That's right. Get yourself over there.", "DEBUG", true, 0, 0)

            currentState = "state3"
            credits = c_getObjectProperty("leapfrog1", 8)
            c_setObjectProperty("leapfrog1", 8, credits + 400)

            setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = "state5"
            
            setupMissionStateScene()
         end

      end

   elseif currentState == "state3" then
      if currentScene == "landing_scene.xml" then
         if actorName == "coDestroyable" and eventId == "BrBR" then
            -- Play dialog
            c_addDialogMessage("Good job, the spring is free. Now destroy the bomb.", "DEBUG", true, 0, 0)
            
            -- Set bomb to more sensitivity
            c_setObjectProperty("bomb1", 0, 2500)
            
            currentState = "state4"
            
            setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = "state5"
            
            setupMissionStateScene()
         end
      end

   elseif currentState == "state4" then
      if currentScene == "landing_scene.xml" then
         if actorName == "bomb1" then
            if eventId == "ExEX" then
               
               -- Play dialog
               c_addDialogMessage("Mission Complete!!!", "DEBUG", true, 0, 0)

               currentState = "state5"
               
               credits = c_getObjectProperty("leapfrog1", 8)
               c_setObjectProperty("leapfrog1", 8, credits + 20)

               setupMissionStateScene()
            end 
         end
      end
   end
end
