-- Mission 2 in space: the stolen stash (lua/mission_2.lua). Launched into the Debris Belt with its exit shut: clear the field until the
-- stash asteroid (stash_asteroid.xml) drifts in, blast it to find the
-- raiders' case (stash_case.xml), shoot it open for the jackpot; the
-- raiders' strippers take it all back and leave fuel to limp home, and the
-- exit opens. Edith is gone when he lands at the silo, her fuel in the tank,
-- and free play starts.

local belt = "debris_belt_scene.xml"

-- Up to the belt
local function toBelt(t)
   t:stashJobTaken()
   t:launch("launchSite1")
   t:expectScene(belt)
end

-- The field is nearly cleared: the stash asteroid has come in
local function toStash(t)
   toBelt(t)
   t:clearField("asteroidField", 3)
end

-- The case is out (the asteroid broken a moment ago)
local function toCase(t)
   toStash(t)
   t:breakObject("stashAsteroid")
   t:wait(1)
end

local function toJackpot(t)
   toCase(t)
   t:breakObject("stashCase")
end

local function toRaid(t)
   toJackpot(t)
   t:wait(16)
end

local function strip(t)
   for _, id in ipairs({ 5, 6, 7, 8 }) do
      t:setProperty("leapfrog1", id, 0)
   end
   t:event("leapfrog1", "LfSt", 7)
end

local function swarmActive(t)
   local s = t.engine:getObject("swarm")
   return s ~= nil and s.props[0] == 1
end

test("stash: the launch goes to the belt: the hunt, no stash yet, no strippers", function(t)
   toBelt(t)
   local S = t:mission(2).State
   t:expectState(S.hunting)
   t:expectSaved("scene", belt)
   t:expectSaved("state", S.hunting)
   t:expectHud("Sector: Debris Belt")
   t:expectDialog("Hurry, get into space!", "Edith")
   t:expectDialog("Clear out some of this junk", "Bruno")
   t:expect(not t.engine:getObject("stashAsteroid"), "no stash asteroid yet")
   t:wait(11)
   t:expectDialog("that one looks loaded", "Edith")
   t:wait(30)
   t:expect(not t.engine:getObject("swarm"), "no strippers while hunting")
end)

test("stash: every corner of the belt is shut during the hunt", function(t)
   toBelt(t)
   local S = t:mission(2).State
   for corner = 1, 4 do
      t:expectCorner(corner, false)
   end
   t:flyToCorner(3)
   t:expectScene(belt)
   t:expectHud("Transit blocked: debris")
   t:expectState(S.hunting)
end)

test("stash: with a few rocks left the stash asteroid drifts in from the edge", function(t)
   toBelt(t)
   local S = t:mission(2).State
   t:clearField("asteroidField", 4)
   t:expectState(S.hunting)
   t:clearField("asteroidField", 3)
   t:expectState(S.stashIn)
   t:expectDialog("drifting in from the edge", "Bruno")
   local rock = t.engine:getObject("stashAsteroid")
   t:expect(rock ~= nil, "the stash asteroid is there")
   t:expect(rock.velocity ~= nil and rock.velocity.x < 0, "it moves in from the right")
   t:expectListening("stashAsteroid", "BrBR", 1)
end)

test("stash: the asteroid blasted: a case inside", function(t)
   toCase(t)
   local S = t:mission(2).State
   t:expectState(S.caseFound)
   t:expectDialog("A box!", "Edith")
   t:expect(t.engine:getObject("stashCase") ~= nil, "the case is out")
   t:expectListening("stashCase", "BrBR", 1)
end)

test("stash: the case shot open: the jackpot, then the raiders after a while", function(t)
   toJackpot(t)
   local S = t:mission(2).State
   t:expectState(S.jackpot)
   t:expectDialog("Look at all that!", "Edith")
   t:expectHud("Stash found")
   t:wait(10)
   t:expect(not t.engine:getObject("swarm"), "time to grab the loot first")
   t:wait(6)
   t:expectState(S.raided)
   t:expectDialog("That's our stash", "Raiders")
   t:expectDialog("From everywhere!", "Edith")
   t:expect(swarmActive(t), "the strippers come")
end)

test("stash: stripped: the strippers leave, fuel to limp home", function(t)
   toRaid(t)
   local m = t:mission(2)
   strip(t)
   t:expectState(m.State.stripped)
   t:expectProperty("leapfrog1", 7, m.limpHomeFuel)
   t:expectDialog("Crawl home.", "Raiders")
   t:expect(not swarmActive(t), "the swarm has stopped")
   t:expectEqual(t.engine:getObject("swarm").recalls, 1, "strippers recalled")
end)

test("stash: the exit stays shut in the raid and opens when he is stripped", function(t)
   toRaid(t)
   t:expectCorner(3, false)
   t:flyToCorner(3)
   t:expectScene(belt)
   strip(t)
   t:expectCorner(3, true)
   t:expectHud("Transit clear")
   t:flyToCorner(3)
   t:expectScene("orbit_scene.xml")
end)

test("stash: home with nothing: Edith on the way down, her fuel at the silo; free play", function(t)
   toRaid(t)
   strip(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectDialog("That went badly, huh?", "Edith")
   t:reentry("onSite", 1)                  -- the village site: the silo, the mission says
   t:expectScene("home_scene.xml")
   t:setProperty("leapfrog1", 7, 3)
   t:land("siloLandingPad")
   t:expectDialog("Gone.", "Leapfrog")
   t:expectDialog("Drained the old pump", "Silo")
   t:expectDialog("whose stash", "Leapfrog")
   t:expectProperty("leapfrog1", 7, t:module("mission2").edithFuel)
   t:expectHud("Mission complete")
   t:expectMission(0)
end)

test("stash: Edith's fuel never takes any away", function(t)
   toRaid(t)
   strip(t)
   t:exitScene(1, 3)
   t:reentry("onSite", 1)
   t:setProperty("leapfrog1", 7, 90)
   t:land("siloLandingPad")
   t:expectProperty("leapfrog1", 7, 90)
end)

test("stash: after the mission, a launch goes to the big space with its strippers", function(t)
   toRaid(t)
   strip(t)
   t:exitScene(1, 3)
   t:reentry("onSite", 1)
   t:land("siloLandingPad")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expect(not t.engine:getObject("stashAsteroid"), "no stash any more")
   t:expect(swarmActive(t), "the world's swarm")
   t:expectCorner(3, true)
end)

test("stash: continuing during the hunt starts a new field", function(t)
   toStash(t)
   local S = t:mission(2).State
   t:restartGame()
   t:expectScene(belt)
   t:expectState(S.hunting)
   t:expect(not t.engine:getObject("stashAsteroid"), "no stash asteroid yet")
   t:clearField("asteroidField", 3)
   t:expectState(S.stashIn)
end)

test("stash: bounced back after the raid: the belt, no strippers, a way home", function(t)
   toRaid(t)
   local S = t:mission(2).State
   strip(t)
   t:exitScene(1, 3)
   t:reentry("bounced")
   t:expectScene(belt)
   t:wait(30)
   t:expectState(S.stripped)
   t:expect(not swarmActive(t), "no strippers")
   t:expectCorner(3, true)
end)

test("stash: a save from the big space before the belt existed is a new hunt", function(t)
   toBelt(t)
   local S = t:mission(2).State
   t:setGlobal("currentState", S.caseFound)
   t:setupScene()
   t:expectState(S.hunting)
end)

test("stash: the field is bigger than what is left when the stash comes", function(t)
   local harness = require("testbench.harness")
   local xml = require("testbench.xml")
   t:newGame()
   local stash = t:mission(2).Stash
   local root = xml.readFile(harness.dataDir .. "/" .. stash.space)
   local props
   for _, c in ipairs(root.children) do
      if c.tag == "system" and c.attr.name == stash.field then
         props = xml.child(c, "objectFactoryProperties")
      end
   end
   t:expect(props ~= nil, stash.field .. " isn't in " .. stash.space)

   local initial = tonumber(props.attr.spawnInitial) or 0
   local intensity = tonumber(props.attr.intensity) or 0
   local total = tonumber(props.attr.spawnTotal) or 0

   -- A field that grows must say how big it gets: the watch waits for that
   if intensity > 0 then
      t:expect(total > 0, stash.field .. " keeps spawning, so it needs a spawnTotal")
      t:expect(total > stash.fieldLeft, "spawnTotal must be more than fieldLeft")
   else
      t:expect(initial > stash.fieldLeft, "spawnInitial must be more than fieldLeft")
   end
end)
