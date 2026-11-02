-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 
--
-- currentMission == 0 means no mission is running (free play). Common
-- (lua/common.lua) and the scene's world (lua/worlds/<world>.lua) run in
-- every case, in that order, before the mission's script.

local common = require('common')
local swarm = require('swarm')
local debt = require('debt')
local mission1 = require('mission1')
local mission2 = require('mission2')

-- The missions by number (currentMission). A mission has
-- setupMissionStateScene, missionStateSceneEventHandler and firstState, and
-- may have reentryOutcome, holds and space. See documentation/lua_guide.md.
local missions = {
   [1] = mission1,
   [2] = mission2,
}

-- The worlds' scripts, and which world each scene belongs to (a world's
-- space and orbit scenes included). A scene not listed has no world script.
local worlds = {
   mining = require('mining'),
}

sceneWorld = {
   ["home_scene.xml"]       = "mining",
   ["landing_scene.xml"]    = "mining",
   ["deep_space_scene.xml"] = "mining",
   ["debris_belt_scene.xml"] = "mining",
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
   if missions[mission] ~= nil then
      return missions[mission].firstState
   end
   return 0
end


-- Global variables
currentState = 0

-- The space scene a launch or a bounce leads to: the running mission's, if
-- it has a space() function that names one, otherwise the big space
function lua_missionSpace()
   local mission = missions[currentMission]
   if mission ~= nil and mission.space ~= nil then
      local space = mission.space()
      if space ~= nil then
         return space
      end
   end
   return "deep_space_scene.xml"
end

-- Ends the running mission and starts another, in the scene the game is in.
-- The save at the next scene exit stores it. Use 0 for free play. Called from
-- an event handler, it happens when all handlers have had that event, so a
-- mission's own handler doesn't go on with the next mission's state.
local handlingEvent = false
local missionToStart = nil

function lua_startMission(mission)
   if handlingEvent then
      missionToStart = mission
      return
   end

   -- The old mission's unanswered questions (missions use ids 1 to 99) go
   -- with it; a world's stay
   for id = 1, 99 do
      c_cancelQuestion(id)
   end

   currentMission = mission
   currentState = lua_missionFirstState(mission)
   lua_setupMissionStateScene()
end

-- missionHolds(what): true if the running mission takes over something the
-- world script would otherwise do in this scene, so the world leaves it
-- alone. what: "swarm" (the world's stripper swarm in deep space). A
-- mission answers with an optional holds(what) function.
function missionHolds(what)
   local mission = missions[currentMission]
   if mission ~= nil and mission.holds ~= nil then
      return mission.holds(what)
   end
   return false
end

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

   local mission = missions[currentMission]
   if mission ~= nil then
      mission.setupMissionStateScene()
   end

   -- Last: the Leapfrog exists by now (debt watches its credits)
   debt.setupScene()

   newGameStarting = false
end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   handlingEvent = true

   debt.eventHandler(eventId, actorName, parameter1)
   common.eventHandler(eventId, actorName, parameter1)
   swarm.eventHandler(eventId, actorName, parameter1)

   local world = currentWorld()
   if world ~= nil then
      world.eventHandler(eventId, actorName, parameter1)
   end

   local mission = missions[currentMission]
   if mission ~= nil then
      mission.missionStateSceneEventHandler(eventId, actorName, parameter1)
   end

   handlingEvent = false

   if missionToStart ~= nil then
      local next = missionToStart
      missionToStart = nil
      lua_startMission(next)
   end
end


-- How a re-entry ended (scene_navigator.lua calls this when the orbit
-- scene exits, before the next scene is chosen and the game is saved, so
-- what it charges is in the save). r is { outcome, siteIndex, siteName,
-- siteScene, distance, burn } as c_lastReentry returns them; outcome is
-- "onSite", "offSite", "bounced" or "burnedUp". The orbit scene's world,
-- then the mission, may handle it with a reentryOutcome function. What
-- they say goes on screen in the next scene (lines added now are dropped
-- with this one).
function lua_reentryOutcome(r)
   local world = currentWorld()
   if world ~= nil and world.reentryOutcome ~= nil then
      world.reentryOutcome(r)
   end

   local mission = missions[currentMission]
   if mission ~= nil and mission.reentryOutcome ~= nil then
      mission.reentryOutcome(r)
   end
end
