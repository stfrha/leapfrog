-- The Baxters' homestead ship, far in the background of a space scene: a
-- sign that they are around. It has no body; it's the sprite "baxterShip"
-- on a parallax layer (0.9) in deep_space_scene.xml, hidden at the start
-- (alpha="0"). Its texture, baxter_homestead (models/sprites/
-- baxter_homestead.py), has two frames: 0 engines burning, 1 cold.
--
--   local baxterShip = require('baxterShip')
--
--   baxterShip.arrive()     -- comes in from far away, slows down, parks;
--                           -- the engines go cold when it has stopped
--   baxterShip.leave()      -- engines on, speeds up and away
--   baxterShip.show()       -- parked, at once (already there when the
--                           -- scene starts)
--   baxterShip.hide()       -- gone, at once
--
-- Each takes an optional shape name last, for a scene with its own ship
-- sprite (default "baxterShip"). arrive and leave take the time in seconds
-- first (nil: the default).
--
-- Call them from the scene setup or an event handler. The scene file is
-- read anew on every scene load, so the ship is hidden again when the
-- Leapfrog comes back: set it up again in the setup for every state where
-- it shall be there (show() if it's already parked).
--
-- mission_script.lua calls baxterShip.setupScene() at every setup and
-- baxterShip.eventHandler for every event (the engines going cold).

local baxterShip = {}

baxterShip.shape = "baxterShip"

-- Where it parks, in stage units as in the scene file (its centre)
baxterShip.park = { x = 1000, y = 230 }

-- Where it comes from and goes to, relative to where it parks, and how
-- small it is there (far away: it grows as it comes closer)
baxterShip.from = { dx = -420, dy = -40, scale = 0.25 }
baxterShip.to = { dx = 520, dy = -70, scale = 0.2 }

baxterShip.arriveSeconds = 14
baxterShip.leaveSeconds = 16

local Frame = { burning = 0, cold = 1 }

-- The engines burn on for this long after it has stopped
local coolDelayTicks = 60 * 3

local coolTimer = -1
local coolShape = nil

local function engines(shape, on)
   c_animateSprite(shape, 0, on and Frame.burning or Frame.cold)
end

function baxterShip.show(shape)
   shape = shape or baxterShip.shape
   coolTimer = -1
   engines(shape, false)
   c_moveSprite(shape, baxterShip.park.x, baxterShip.park.y, 1, 1, 0)
end

function baxterShip.hide(shape)
   shape = shape or baxterShip.shape
   coolTimer = -1
   c_moveSprite(shape, nil, nil, nil, 0, 0)
end

function baxterShip.arrive(seconds, shape)
   shape = shape or baxterShip.shape
   seconds = seconds or baxterShip.arriveSeconds
   local p, f = baxterShip.park, baxterShip.from
   engines(shape, true)
   c_moveSprite(shape, p.x + f.dx, p.y + f.dy, f.scale, 0, 0)
   c_moveSprite(shape, p.x, p.y, 1, 1, seconds, "out")
   coolShape = shape
   coolTimer = c_startSceneTimer(math.floor(seconds * 60) + coolDelayTicks)
end

function baxterShip.leave(seconds, shape)
   shape = shape or baxterShip.shape
   seconds = seconds or baxterShip.leaveSeconds
   local p, t = baxterShip.park, baxterShip.to
   coolTimer = -1
   engines(shape, true)
   c_moveSprite(shape, p.x + t.dx, p.y + t.dy, t.scale, 0, seconds, "in")
end

-- A timer from an earlier scene must not match a new one
function baxterShip.setupScene()
   coolTimer = -1
end

function baxterShip.eventHandler(eventId, actorName, parameter1)
   if eventId == "ScTO" and coolTimer >= 0 and parameter1 == coolTimer then
      coolTimer = -1
      engines(coolShape, false)
   end
end

return baxterShip
