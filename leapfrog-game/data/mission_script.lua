-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 
--
-- currentMission == 0 means no mission is running (free play). Common
-- (lua/common.lua) and the scene's world (lua/worlds/<world>.lua) run in
-- every case, in that order, before the mission's script.

local common = require('common')
local swarm = require('swarm')
local mission1 = require('mission1')

-- The worlds' scripts, and which world each scene belongs to (a world's
-- space and orbit scenes included). A scene not listed has no world script.
local worlds = {
   mining = require('mining'),
}

local sceneWorld = {
   ["home_scene.xml"]       = "mining",
   ["landing_scene.xml"]    = "mining",
   ["deep_space_scene.xml"] = "mining",
   ["orbit_scene.xml"]      = "mining",
}

local function currentWorld()
   local name = sceneWorld[currentScene]
   if name ~= nil then
      return worlds[name]
   end
   return nil
end

-- The state a mission starts in (each mission declares its states by name)
function lua_missionFirstState(mission)
   if mission == 1 then
      return mission1.firstState
   end
   return 0
end


-- Global variables
currentState = 0

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

   common.setupScene()
   swarm.setupScene()

   local world = currentWorld()
   if world ~= nil then
      world.setupScene()
   end

   if currentMission == 1 then
      mission1.setupMissionStateScene()
   end

   newGameStarting = false
end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   common.eventHandler(eventId, actorName, parameter1)
   swarm.eventHandler(eventId, actorName, parameter1)

   local world = currentWorld()
   if world ~= nil then
      world.eventHandler(eventId, actorName, parameter1)
   end

   if currentMission == 1 then
      mission1.missionStateSceneEventHandler(eventId, actorName, parameter1)
   end
end









