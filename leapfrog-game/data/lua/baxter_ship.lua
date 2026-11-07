-- The Baxters' homestead ship, far in the background of a space scene: a
-- sign that they are around. It has no body; it's the sprite "baxterShip"
-- in deep_space_scene.xml, hidden at the start (alpha="0"). It sits in the
-- scene itself (parallaxAmount="0", zLevel 0, behind everything but the
-- stars), so it stays put while the Leapfrog flies; small and hazy (the
-- sprite's colour, and the alpha baxterShip.haze below) so it reads as far
-- away. (On a parallax layer it followed the Leapfrog about.) Its texture, baxter_homestead (models/sprites/
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
--   baxterShip.tag(name)    -- which tag is sprayed on the green hangar's
--                           -- doors: "baxterTag" (BXTR, the default), the
--                           -- shape of another tag, or nil for none. It
--                           -- shows from the next show() or arrive(); to
--                           -- change it on a ship parked in view, call
--                           -- show() after it
--
-- Each of the first four takes an optional ship shape name last, for a
-- scene with its own ship sprite (default "baxterShip"). arrive and leave
-- take the time in seconds first (nil: the default).
--
-- Tags are overlay sprites in the scene, on the same layer as the ship
-- (and the same colour),
-- hidden at the start (alpha="0", zLevel above the ship). They move,
-- scale and fade with the ship, at their offset from its centre (times
-- its scale), so they stay on the doors. A new tag: draw it (transparent
-- around it, about 2:1), add the texture to res_space.xml, add a sprite
-- box to the scene like baxterTag, and add it to overlays below with the
-- same offset. The offset and door size are printed by
-- baxter_homestead.py after each render. The tag chosen is kept across
-- scenes until changed.
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

-- Where it parks, in scene units as in the scene file (its centre), in the
-- big space
baxterShip.park = { x = 1000, y = 230 }

-- Where it parks in each scene that has the sprites, and its size there
-- relative to a 480 x 120 ship (the sprites in that scene file are that
-- size; the travel and the overlays' offsets scale with it)
baxterShip.places = {
   ["deep_space_scene.xml"]  = { x = baxterShip.park.x, y = baxterShip.park.y, size = 0.5 },   -- 240 x 60
   ["debris_belt_scene.xml"] = { x = 330, y = 90, size = 0.25 },                                 -- 120 x 30
}

-- How much of it shows when it's there: less than 1 lets the stars through,
-- a haze that puts it far away
baxterShip.haze = 0.7

local function place()
   return baxterShip.places[currentScene] or { x = baxterShip.park.x, y = baxterShip.park.y, size = 0.5 }
end

-- Where it comes from and goes to, relative to where it parks, and how
-- small it is there (far away: it grows as it comes closer)
baxterShip.from = { dx = -420, dy = -40, scale = 0.25 }
baxterShip.to = { dx = 520, dy = -70, scale = 0.2 }

baxterShip.arriveSeconds = 14
baxterShip.leaveSeconds = 16

-- Overlay sprites per ship sprite, at their offset from its centre (units
-- of a 480 x 120 ship at scale 1, y down)
baxterShip.overlays = {
   baxterShip = {
      baxterTag = { dx = -66.6, dy = 20.0 },
   },
}

local shownTag = "baxterTag"

local Frame = { burning = 0, cold = 1 }

-- The engines burn on for this long after it has stopped
local coolDelayTicks = 60 * 3

local coolTimer = -1
local coolShape = nil

local function engines(shape, on)
   c_animateSprite(shape, 0, on and Frame.burning or Frame.cold)
end

-- Moves the ship and its overlays; the tag shown follows the ship's alpha
local function move(shape, x, y, scale, alpha, seconds, ease)
   local size = place().size
   c_moveSprite(shape, x, y, scale, alpha, seconds, ease)
   for overlay, o in pairs(baxterShip.overlays[shape] or {}) do
      local ox, oy = nil, nil
      if x ~= nil then
         ox = x + o.dx * scale * size
         oy = y + o.dy * scale * size
      end
      local a = alpha
      if a ~= nil and overlay ~= shownTag then
         a = 0
      end
      c_moveSprite(overlay, ox, oy, scale, a, seconds, ease)
   end
end

function baxterShip.show(shape)
   shape = shape or baxterShip.shape
   coolTimer = -1
   engines(shape, false)
   local p = place()
   move(shape, p.x, p.y, 1, baxterShip.haze, 0)
end

function baxterShip.hide(shape)
   shape = shape or baxterShip.shape
   coolTimer = -1
   move(shape, nil, nil, nil, 0, 0)
end

function baxterShip.arrive(seconds, shape)
   shape = shape or baxterShip.shape
   seconds = seconds or baxterShip.arriveSeconds
   local p, f = place(), baxterShip.from
   engines(shape, true)
   move(shape, p.x + f.dx * p.size, p.y + f.dy * p.size, f.scale, 0, 0)
   move(shape, p.x, p.y, 1, baxterShip.haze, seconds, "out")
   coolShape = shape
   coolTimer = c_startSceneTimer(math.floor(seconds * 60) + coolDelayTicks)
end

function baxterShip.leave(seconds, shape)
   shape = shape or baxterShip.shape
   seconds = seconds or baxterShip.leaveSeconds
   local p, t = place(), baxterShip.to
   coolTimer = -1
   engines(shape, true)
   move(shape, p.x + t.dx * p.size, p.y + t.dy * p.size, t.scale, 0, seconds, "in")
end

-- Which tag shows on the doors (an overlay's shape, or nil for none),
-- from the next show() or arrive()
function baxterShip.tag(name)
   shownTag = name
end

function baxterShip.shownTag()
   return shownTag
end

-- A new scene starts with everything hidden; a timer from an earlier scene
-- must not match a new one
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
