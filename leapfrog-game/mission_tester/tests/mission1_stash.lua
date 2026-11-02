-- Mission 1 in space: the stolen stash (lua/mission_1.lua). Launched from
-- the Orb: find Bruno's asteroid (stash_asteroid.xml), blast it to find the
-- raiders' case (stash_case.xml), shoot it open for the jackpot; the
-- raiders' strippers take it all back, leave fuel to limp home; Edith is
-- gone when he lands at the silo.

-- Mission 1 up to deep space: the job, the Orb, the launch
local function toDeepSpace(t)
   t:newGame()
   t:land("siloLandingPad")
   t:answer(t:mission(1).Question.acceptJob, true)
   t:takeOff()
   t:land("oreLanding")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
end

-- The case is out (the asteroid broken a moment ago)
local function toCase(t)
   toDeepSpace(t)
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

test("stash: the launch starts the hunt: Bruno's hint, the stash asteroid, no strippers", function(t)
   toDeepSpace(t)
   local S = t:mission(1).State
   t:expectState(S.hunting)
   t:expectSaved("state", S.hunting)
   t:expectDialog("Hurry, get into space!", "Edith")
   t:expectDialog("big one, out past the station", "Bruno")
   t:expect(t.engine:getObject("stashAsteroid") ~= nil, "the stash asteroid is there")
   t:expectListening("stashAsteroid", "BrBR", 1)
   t:wait(11)
   t:expectDialog("that one looks loaded", "Edith")
   t:wait(30)
   t:expect(not t.engine:getObject("swarm"), "no strippers while hunting")
end)

test("stash: the asteroid blasted: a case inside", function(t)
   toCase(t)
   local S = t:mission(1).State
   t:expectState(S.caseFound)
   t:expectDialog("A box!", "Edith")
   t:expect(t.engine:getObject("stashCase") ~= nil, "the case is out")
   t:expectListening("stashCase", "BrBR", 1)
end)

test("stash: the case shot open: the jackpot, then the raiders after a while", function(t)
   toJackpot(t)
   local S = t:mission(1).State
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
   local m = t:mission(1)
   strip(t)
   t:expectState(m.State.stripped)
   t:expectProperty("leapfrog1", 7, m.limpHomeFuel)
   t:expectDialog("Crawl home.", "Raiders")
   t:expect(not swarmActive(t), "the swarm has stopped")
   t:expectEqual(t.engine:getObject("swarm").recalls, 1, "strippers recalled")
end)

test("stash: home with nothing: Edith on the way down, gone at the silo; mission complete", function(t)
   toRaid(t)
   local S = t:mission(1).State
   strip(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectDialog("That went badly, huh?", "Edith")
   t:reentry("onSite", 0)                  -- the gold mine site: home for now
   t:expectScene("home_scene.xml")
   t:land("siloLandingPad")
   t:expectState(S.done)
   t:expectDialog("Gone.", "Leapfrog")
   t:expectDialog("whose stash", "Leapfrog")
   t:expectHud("Mission complete")
end)

test("stash: after the mission the strippers are back in deep space", function(t)
   toRaid(t)
   local S = t:mission(1).State
   strip(t)
   t:exitScene(1, 3)
   t:reentry("onSite", 0)
   t:land("siloLandingPad")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectState(S.done)
   t:expect(not t.engine:getObject("stashAsteroid"), "no stash any more")
   t:expect(swarmActive(t), "the world's swarm")
end)

test("stash: fleeing to orbit with the loot: the clinging strippers take it on the way", function(t)
   toRaid(t)
   local m = t:mission(1)
   t:setProperty("leapfrog1", 8, 500)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectState(m.State.stripped)
   t:expectDialog("still on your hull", "Raiders")
   t:expectProperty("leapfrog1", 8, 0)
   t:expectProperty("leapfrog1", 7, m.limpHomeFuel)
end)

test("stash: continuing during the hunt: the stash asteroid is there again", function(t)
   toDeepSpace(t)
   local S = t:mission(1).State
   t:restartGame()
   t:expectScene("deep_space_scene.xml")
   t:expectState(S.hunting)
   t:expect(t.engine:getObject("stashAsteroid") ~= nil, "the stash asteroid is there")
end)

test("stash: back in deep space with the case found: the case is there again", function(t)
   toCase(t)
   local S = t:mission(1).State
   t:exitScene(1, 3)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:expectState(S.caseFound)
   t:expect(t.engine:getObject("stashCase") ~= nil, "the case is there")
   t:breakObject("stashCase")
   t:expectState(S.jackpot)
end)

test("stash: back in deep space with the loot out: the raiders come at once", function(t)
   toJackpot(t)
   local S = t:mission(1).State
   t:exitScene(1, 3)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:wait(4)
   t:expectState(S.raided)
   t:expect(swarmActive(t), "the strippers come")
end)

test("stash: bounced back after the raid: no strippers, a way home", function(t)
   toRaid(t)
   local S = t:mission(1).State
   strip(t)
   t:exitScene(1, 3)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:wait(30)
   t:expectState(S.stripped)
   t:expect(not swarmActive(t), "no strippers")
end)

test("stash: saved at the launch before this part existed (atOrb): the hunt begins", function(t)
   toDeepSpace(t)
   local S = t:mission(1).State
   t:setGlobal("currentState", S.atOrb)
   t:exitScene(1, 3)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:expectState(S.hunting)
   t:expect(t.engine:getObject("stashAsteroid") ~= nil, "the stash asteroid is there")
end)
