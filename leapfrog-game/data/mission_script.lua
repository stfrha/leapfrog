-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.

-- Global variables
currentState = 1
firstSceneOfToday = true


-- This function loads all scene objects that match the 
-- mission state. This includes the leapfrog at the position
-- of the state in this scene. 
function lua_setupInitialMissionStateScene()

   print("Hello hello hello hello hello hello hello hello!!!")
   
   if currentScene == "landing_scene.xml" then
      if currentMission == 1 then
         if currentState == 1 then
            -- Initialise leapfrog as descenting
            c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 415, 657)

            -- Reconfigure leapfrog to landing mode
            c_setObjectProperty("leapfrog1", 0, 2)
            
         elseif currentState == 2 then

         end
      end
   end
end

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function lua_setupMissionStateScene()

   if firstSceneOfToday then
      
      firstSceneOfToday = false
      
      lua_setupInitialMissionStateScene()
      
   end 

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()

   -- Regardless of state, we always want to listen to leapfrog has landed 
   -- on the launch site
   if currentScene == "landing_scene.xml" then
         -- Register event for landing on launchsite 1
         c_registerEventHandler("launchSite1", "LsLL", 0, 0)
   end

   if currentState == 1 then
   
      if currentScene == "landing_scene.xml" then

         c_addMissionStateSceneObjects("landing_scene_state2.xml")

         -- Start timer, will send ScTO event when timed out 
         state1Timer = c_startSceneTimer(240)

         -- Register event for landing on landingPad1
         c_registerEventHandler("landing_pad1", "LpLL", 0, 0)

         -- Detect if bomb exploded prematurely
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 1)

         -- Play dialog
         c_addDialogMessage("Hello. Try to land on the landing pad, right below you.", "DEBUG", true, 0, 0)
      end

   elseif currentState == 2 then
   
      if currentScene == "landing_scene.xml" then
         
         -- Register event for leaving (Take-off) landingPad1
         c_registerEventHandler("landing_pad1", "LpTO", 0, 0)
         
         -- Detect if bomb exploded prematurely
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

         -- Set state of next mission objective
         c_setObjectMapState("coDestroyable", "springDestroyable", 0)

         -- Set bomb to be very in-sensitivity
         c_setObjectProperty("bomb1", 0, 250000)

      end

   elseif currentState == 3 then
      if currentScene == "landing_scene.xml" then
         c_registerEventHandler("coDestroyable", "BrBR", 0, 0)
         c_registerEventHandler("bomb1", "ExEX", 0, 0)
      end

   elseif currentState == 4 then
      if currentScene == "landing_scene.xml" then
         c_registerEventHandler("bomb1", "ExEX", 0, 0)
      end


   end
end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)

   -- Regardless of state, we always want to listen to leapfrog has landed 
   -- on the launch site
   if currentScene == "landing_scene.xml" then
         if eventId == "LsLL" then
            c_addDialogMessage("You will now be launched into deep space.", "DEBUG", true, 0, 0)
            c_setObjectProperty("launchSite1", 1, 0)
         end
   end


   if currentState == 1 then
      if currentScene == "landing_scene.xml" then
         if eventId == "ScTO" and parameter1 == state1Timer then

            -- Play dialog
            c_addDialogMessage("The landing pad is the filled square on your map. Get there!", "DEBUG", true, 0, 0)
            c_addPositionedChildObject("medium_asteroid.xml", "dummy1", 500, 500)

         end

         if actorName == "landing_pad1" and eventId == "LpLL" then
            
            -- Play dialog
            c_addDialogMessage("Yippie: Welcome to the landing pad. Let me fill you up. A new obstacle has arrived in the grotto. Check it out.", "DEBUG", true, 0, 0)

            currentState = 2
            
            lua_setupMissionStateScene()
         end

         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 5
            
            lua_setupMissionStateScene()
         end

      end

   elseif currentState == 2 then
      if currentScene == "landing_scene.xml" then
         if actorName == "landing_pad1" and eventId == "LpTO" then
               
            -- Play dialog
            c_addDialogMessage("That's right. Get yourself over there.", "DEBUG", true, 0, 0)

            currentState = 3

            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 5
            
            lua_setupMissionStateScene()
         end

      end

   elseif currentState == 3 then
      if currentScene == "landing_scene.xml" then
         if actorName == "coDestroyable" and eventId == "BrBR" then
            -- Play dialog
            c_addDialogMessage("Good job, the spring is free. Now destroy the bomb.", "DEBUG", true, 0, 0)
            
            -- Set bomb to more sensitivity
            c_setObjectProperty("bomb1", 0, 2500)
            
            currentState = 4
            
            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 5
            
            lua_setupMissionStateScene()
         end
      end

   elseif currentState == 4 then
      if currentScene == "landing_scene.xml" then
         if actorName == "bomb1" then
            if eventId == "ExEX" then
               
               -- Play dialog
               c_addDialogMessage("Mission Complete!!!", "DEBUG", true, 0, 0)

               currentState = 5
               
               credits = c_getObjectProperty("leapfrog1", 8)
               c_setObjectProperty("leapfrog1", 8, credits + 20)

               lua_setupMissionStateScene()
            end 
         end
      end
   end
end
