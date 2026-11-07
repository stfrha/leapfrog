-- The shield power unit on the station's boom (lua/worlds/mining.lua,
-- data/docking_station.xml), there while the Baxters hold the station
-- (mission "halfway")

local harness = require("testbench.harness")
local xml = require("testbench.xml")

-- DockingStation's defaults: on the station body, in its coordinates
local shieldX, shieldY, shieldRadius = 13, -1, 38

-- Half the unit's box
local unitHalfWidth, unitHalfHeight = 13, 4

-- Where the station stands in the scene file
local function stationPosition()
   local root = xml.readFile(harness.dataDir .. "/deep_space_scene.xml")
   for _, c in ipairs(root.children) do
      if c.tag == "childObject" and c.attr.name == "dockingStation1" then
         return tonumber(c.attr.posX), tonumber(c.attr.posY)
      end
   end
end

-- A child of docking_station.xml by tag and name
local function stationPart(tag, name)
   local root = xml.readFile(harness.dataDir .. "/docking_station.xml")
   for _, c in ipairs(root.children) do
      if c.tag == tag and c.attr.name == name then
         return c.attr
      end
   end
end

-- The centre of the station's shield in the scene
local function shieldCentre()
   local sx, sy = stationPosition()
   local base = stationPart("dynamicBox", "stationBase")
   return sx + tonumber(base.posX) + shieldX, sy + tonumber(base.posY) + shieldY
end

-- The boom's centre in the scene
local function boomPosition()
   local sx, sy = stationPosition()
   local boom = stationPart("dynamicBox", "powerBoom")
   return sx + tonumber(boom.posX), sy + tonumber(boom.posY)
end

-- The big space with the station held (the test start "Space, station held")
local function toBigSpace(t)
   t:newGame()
   for i, label in ipairs(t:testStarts()) do
      if label == "Space, station held" then
         t:menuTestStart(i)
      end
   end
   t:expectScene("deep_space_scene.xml")
   t:expectMission("halfway")
end

test("shield power: not there unless a mission puts it there", function(t)
   t:newGame()
   t:menuTestStart(2)                      -- the big space, the old test mission
   t:expectScene("deep_space_scene.xml")
   t:expect(t.engine:getObject(t:module("mining").ShieldPower.object) == nil, "no unit")
end)

test("shield power: in the big space, welded to the station's boom", function(t)
   toBigSpace(t)
   local p = t:module("mining").ShieldPower
   t:expect(t.engine:getObject(p.object) ~= nil, "the unit is there")
   local welds = t.engine.scene.welds
   t:expectEqual(welds and #welds, 1, "welds")
   t:expectEqual(welds[1].a, p.boom, "fastened to the boom")
   t:expectEqual(welds[1].b, p.body, "and to the unit")
end)

test("shield power: a new setup in the same scene doesn't add it again", function(t)
   toBigSpace(t)
   local function added()
      local n = 0
      for _, line in ipairs(t.engine.log) do
         if line.kind == "OBJECTS" and line.text:find("shield_power.xml", 1, true) then n = n + 1 end
      end
      return n
   end
   local before = added()
   t.engine:callLua("lua_setupMissionStateScene")
   t:expectEqual(added(), before, "units added")
end)

test("shield power: placed so that the weld's anchors meet", function(t)
   toBigSpace(t)
   local p = t:module("mining").ShieldPower
   local bx, by = boomPosition()
   t:expect(math.abs(bx + p.boomAnchor.x - (p.x + p.unitAnchor.x)) < 0.01, "x")
   t:expect(math.abs(by + p.boomAnchor.y - (p.y + p.unitAnchor.y)) < 0.01, "y")
end)

test("shield power: the boom's weld on the station lines up", function(t)
   local weld = stationPart("weldJoint", "boomWeld")
   local base = stationPart("dynamicBox", "stationBase")
   local boom = stationPart("dynamicBox", "powerBoom")
   local ax = tonumber(base.posX) + tonumber(weld.objectAAnchorX)
   local ay = tonumber(base.posY) + tonumber(weld.objectAAnchorY)
   local bx = tonumber(boom.posX) + tonumber(weld.objectBAnchorX)
   local by = tonumber(boom.posY) + tonumber(weld.objectBAnchorY)
   t:expect(math.abs(ax - bx) < 0.01, "x")
   t:expect(math.abs(ay - by) < 0.01, "y")
end)

test("shield power: all of it outside the station's shield", function(t)
   toBigSpace(t)
   local p = t:module("mining").ShieldPower
   local cx, cy = shieldCentre()
   -- The box's point nearest the shield's centre
   local nx = math.max(p.x - unitHalfWidth, math.min(cx, p.x + unitHalfWidth))
   local ny = math.max(p.y - unitHalfHeight, math.min(cy, p.y + unitHalfHeight))
   local distance = math.sqrt((cx - nx) ^ 2 + (cy - ny) ^ 2)
   t:expect(distance > shieldRadius, string.format("nearest point %.1f from the centre", distance))
end)
