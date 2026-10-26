-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 
--
-- currentMission == 0 means no mission is running (free play). The world
-- script (lua/world.lua) runs in every case, before the mission's.

local world = require('world')
local mission1 = require('mission1')


-- Global variables
currentState = 1

-- Set by lua_startInitialScene when a new game starts, so the mission can
-- play its opening; cleared once the first scene has been set up
newGameStarting = false

-- Events registered since the last c_clearAllTriggersAndEvents, so that
-- the world and a mission listening to the same event register it once
local registeredEvents = {}

-- listen(actorName, eventId): register for an event (with
-- c_registerEventHandler) unless already registered since the last clear
function listen(actorName, eventId)
   local key = actorName .. "/" .. eventId
   if registeredEvents[key] == nil then
      registeredEvents[key] = true
      c_registerEventHandler(actorName, eventId, 0, 0)
   end
end

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
         c_setObjectProperty("leapfrog1", 0, nextLfMode)
         lfDefined = true      
      end
   end

   -- Clear triggers and event handlers: only here, so the world and the
   -- mission don't wipe each other's
   c_clearAllTriggersAndEvents()
   registeredEvents = {}

   world.setupScene()

   if currentMission == 1 then
      mission1.setupMissionStateScene()
   end

   newGameStarting = false
end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   world.eventHandler(eventId, actorName, parameter1)

   if currentMission == 1 then
      mission1.missionStateSceneEventHandler(eventId, actorName, parameter1)
   end
end









