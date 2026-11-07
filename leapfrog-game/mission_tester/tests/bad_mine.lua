-- The bad mine (lua/worlds/mining.lua): a hidden landing site whose
-- re-entry solution (key item badMineSolution) the station master hands
-- over when the station is taken back (mission "halfway",
-- mining.giveBadMineSolution); then the orbit shows the site. (Needs <landingSite name="badMine" hidden="true"> in
-- mining_planet_surface.xml.)

-- The big space, where the station is
local function toDeepSpace(t)
   t:newGame()
   t:menuTestStart(2)                      -- the big space, launched
   t:expectScene("deep_space_scene.xml")
   t:clearDialog()
end

-- The handover (what halfway does once the station is his)
local function handOver(t)
   return t:module("mining").giveBadMineSolution()
end

-- The index of a landing site of the orbit scene (file order, from 0)
local function siteIndex(t, name)
   for i, site in ipairs(t.engine.scene.sites or {}) do
      if site.name == name then
         return i - 1, site
      end
   end
   t:fail("the orbit scene has no landing site " .. name)
end

test("bad mine: the handover gives its solution, once", function(t)
   toDeepSpace(t)
   t:expectItem("badMineSolution", "unknown")
   t:expect(handOver(t), "given")
   t:expectItem("badMineSolution", "held")
   t:expectHud("Landing solution received: Bad Mine solution")
   t:expect(not handOver(t), "not again")
end)

test("bad mine: docking at the station gives nothing by itself", function(t)
   toDeepSpace(t)
   t:event("dockingStation1", "DsDk", 0)
   t:expectItem("badMineSolution", "unknown")
end)

test("bad mine: hidden in orbit without the solution", function(t)
   toDeepSpace(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectLandingSiteShown("badMine", false)
   t:expectLandingSiteShown("goldMine", true)
end)

test("bad mine: shown in orbit with the solution; coming down there leads to its scene", function(t)
   toDeepSpace(t)
   handOver(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectLandingSiteShown("badMine", true)
   local index, site = siteIndex(t, "badMine")
   t:reentry("onSite", index)
   t:expectScene((site.scene ~= "") and site.scene or "landing_scene.xml")
end)

test("bad mine: the solution is kept after saving and continuing", function(t)
   toDeepSpace(t)
   handOver(t)
   t:exitScene(1, 3)
   t:restartGame()
   t:expectScene("orbit_scene.xml")
   t:expectItem("badMineSolution", "held")
   t:expectLandingSiteShown("badMine", true)
end)

test("bad mine: a revoked solution hides the site and isn't handed over again", function(t)
   toDeepSpace(t)
   handOver(t)
   t:revokeItem("badMineSolution")
   t:expect(not handOver(t), "not handed over again")
   t:expectItem("badMineSolution", "revoked")
   t:exitScene(1, 3)
   t:expectLandingSiteShown("badMine", false)
end)
