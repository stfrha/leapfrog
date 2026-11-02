-- The bad mine (lua/worlds/mining.lua): a hidden landing site whose
-- re-entry solution (key item badMineSolution) the docking station in deep
-- space hands over the first time the Leapfrog docks; then the orbit shows
-- the site. (Needs <landingSite name="badMine" hidden="true"> in
-- mining_planet_surface.xml.)

-- The big space, where the station is
local function toDeepSpace(t)
   t:newGame()
   t:menuTestStart(2)                      -- the big space, launched
   t:expectScene("deep_space_scene.xml")
   t:clearDialog()
end

local function dock(t)
   t:event("dockingStation1", "DsDk", 0)
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

test("bad mine: docking at the station hands over its solution, once", function(t)
   toDeepSpace(t)
   t:expectItem("badMineSolution", "unknown")
   dock(t)
   t:expectDialog("re-entry solution for an old mine", "Station")
   t:expectItem("badMineSolution", "held")
   t:expectHud("Landing solution received: Bad Mine solution")
   t:clearDialog()
   dock(t)
   t:expectNoDialog("re-entry solution")
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
   dock(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectLandingSiteShown("badMine", true)
   local index, site = siteIndex(t, "badMine")
   t:reentry("onSite", index)
   t:expectScene((site.scene ~= "") and site.scene or "landing_scene.xml")
end)

test("bad mine: the solution is kept after saving and continuing", function(t)
   toDeepSpace(t)
   dock(t)
   t:exitScene(1, 3)
   t:restartGame()
   t:expectScene("orbit_scene.xml")
   t:expectItem("badMineSolution", "held")
   t:expectLandingSiteShown("badMine", true)
end)

test("bad mine: a revoked solution hides the site and isn't handed over again", function(t)
   toDeepSpace(t)
   dock(t)
   t:revokeItem("badMineSolution")
   t:clearDialog()
   dock(t)
   t:expectNoDialog("re-entry solution")
   t:expectItem("badMineSolution", "revoked")
   t:exitScene(1, 3)
   t:expectLandingSiteShown("badMine", false)
end)
