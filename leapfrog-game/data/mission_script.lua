-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 

-- Global variables
currentState = 1
firstSceneOfToday = true
lfDefined = false

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function lua_setupMissionStateScene()

   -- Test if LF shall be positioned according to the nextLfxxxx
   -- variables
   if useInitialLfPos then
      -- Note that if the file loaded is not the same configuration as 
      -- the nextLfConfiguration, the LF will reconfigure as an animation
      c_addPositionedChildObject(nextLfFile, "leapfrog1", nextLfPosX, nextLfPosY)
      c_setObjectProperty("leapfrog1", 0, nextLfConfiguration)
      lfDefined = true      
   end

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()
   
   if currentMission == 1 then
      local mission = require 'mission_1'
   end

   mission.setupMissionStateScene()

end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   mission.missionStateSceneEventHandler(eventId, actorName, parameter1)
end









