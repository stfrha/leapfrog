-- Swarms of strippers (behaviour "swarm", see src/swarm.h and
-- src/stripper.h): start one in a scene, and have it stopped when the
-- strippers have taken everything.
--
--   local swarm = require('swarm')
--
--   -- in the mission's scene setup, for the state where they attack:
--   swarm.start("swarm", "stripper_swarm.xml", function(name)
--      -- the Leapfrog has been stripped: the swarm has stopped spawning
--      -- and its strippers are leaving. Go on with the story here.
--   end)
--
-- The strippers send LfSt (from leapfrog1) for every theft, and with
-- parameter 0 when they found nothing to take. On each one, the Leapfrog's
-- resources are checked; when all of them are empty, the swarm is stopped
-- and its live strippers are recalled. (Upgrades: when they exist, add a
-- check for them in swarm.isStripped.)
--
-- mission_script.lua calls swarm.setupScene() at every scene setup (the
-- swarm objects are gone with the old scene) and swarm.eventHandler for
-- every event, before the mission's own handler.

local swarm = {}

-- The swarm's properties (c_setObjectProperty / c_getObjectProperty)
local Prop = {
   active   = 0,   -- 1 spawning, 0 stopped
   rate     = 1,   -- spawns per second, now (setting it ends the ramp)
   maxAlive = 2,   -- most alive at a time, now (setting it ends the ramp)
   alive    = 3,   -- read only
   spawned  = 4,   -- read only
   recall   = 5,   -- write 1: the live strippers leave
}
swarm.Prop = Prop

-- The Leapfrog's resources a stripper can take (property ids). What a
-- stripped Leapfrog means by default: all four empty.
swarm.allResources = { 5, 6, 7, 8 }   -- ammo, shield, fuel, credits

local leapfrog = "leapfrog1"

-- Swarms running in this scene: name -> { onStripped, resources }
local running = {}

-- True if every resource in the list (default: all four) is empty
function swarm.isStripped(resources)
   for _, id in ipairs(resources or swarm.allResources) do
      if c_getObjectProperty(leapfrog, id) > 0 then
         return false
      end
   end
   return true
end

-- swarm.start(name, file [, onStripped [, resources]])
-- Adds the swarm in file (e.g. "stripper_swarm.xml") to the scene as
-- name. When the Leapfrog's resources in the list (default: all four) are
-- empty, it is stopped, its strippers are recalled, and onStripped(name)
-- is called (once).
function swarm.start(name, file, onStripped, resources)
   c_addPositionedChildObject(file, name, 0, 0)
   running[name] = { onStripped = onStripped, resources = resources }
   listen(leapfrog, "LfSt")
end

-- Stops spawning; with recall, the live strippers leave too
function swarm.stop(name, recall)
   c_setObjectProperty(name, Prop.active, 0)
   if recall then
      c_setObjectProperty(name, Prop.recall, 1)
   end
   running[name] = nil
end

-- Harder or easier from now on (ends the swarm's own ramp)
function swarm.set(name, rate, maxAlive)
   if rate ~= nil then
      c_setObjectProperty(name, Prop.rate, rate)
   end
   if maxAlive ~= nil then
      c_setObjectProperty(name, Prop.maxAlive, maxAlive)
   end
end

function swarm.isRunning(name)
   return running[name] ~= nil
end

swarm.setupScene = function()
   running = {}
end

swarm.eventHandler = function(eventId, actorName, parameter1)
   if eventId ~= "LfSt" or actorName ~= leapfrog then
      return
   end

   -- Copy the names first: a callback may start or stop swarms
   local names = {}
   for name, _ in pairs(running) do
      names[#names + 1] = name
   end

   for _, name in ipairs(names) do
      local s = running[name]
      if s ~= nil and swarm.isStripped(s.resources) then
         swarm.stop(name, true)
         if s.onStripped ~= nil then
            s.onStripped(name)
         end
      end
   end
end

return swarm
