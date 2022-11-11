-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 

package.path = package.path .. ";./assets/?.lua"
local mission1 = require 'mission_1'

-- Global variables
currentState = 1

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function lua_setupMissionStateScene()

   -- Test if LF shall be positioned according to the nextLfxxxx
   -- variables
   if lfDefined == false then
      if useInitialLfPos == false then
         -- Note that if the file loaded is not the same configuration as 
         -- the nextLfMode, the LF will reconfigure as an animation
         c_addPositionedChildObject(nextLfFile, "leapfrog1", nextLfPosX, nextLfPosY)
         c_setObjectProperty("leapfrog1", 1, nextLfMode)
         lfDefined = true      
      end
   end

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()
   if currentMission == 1 then
      mission1.setupMissionStateScene()
   end

end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   if currentMission == 1 then
      mission1.missionStateSceneEventHandler(eventId, actorName, parameter1)
   end
end









