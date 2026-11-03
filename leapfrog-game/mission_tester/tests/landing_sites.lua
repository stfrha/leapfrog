-- Hidden landing sites: hidden="true" in the planet's surface file, shown
-- by the scripts with c_setLandingSiteShown in the orbit scene's setup.
-- (The real surface file has no hidden site; hideLandingSite marks one in
-- the test's copy.)

-- Mission 1 up to the orbit scene (as in reentry.lua)
local function toOrbit(t)
   t:newGame()
   t:menuTestStart(2)                      -- the big space, launched
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:clearDialog()
end

test("landing sites: shown unless hidden in the surface file", function(t)
   toOrbit(t)
   t:expectLandingSiteShown("goldMine", true)
   t:expectLandingSiteShown("alphaCity", true)
end)

test("landing sites: a hidden site starts hidden; shown, re-entry can come down there", function(t)
   t:hideLandingSite("alphaCity")
   toOrbit(t)
   t:expectLandingSiteShown("goldMine", true)
   t:expectLandingSiteShown("alphaCity", false)
   t:setLandingSiteShown("alphaCity", true)
   t:expectLandingSiteShown("alphaCity", true)
   t:reentry("onSite", 1)
   t:expectScene("home_scene.xml")
end)

test("landing sites: a script can hide a site", function(t)
   toOrbit(t)
   t:setLandingSiteShown("goldMine", false)
   t:expectLandingSiteShown("goldMine", false)
   t:reentry("onSite", 1)
   t:expectScene("home_scene.xml")
end)

test("landing sites: hidden again in the next visit of the orbit scene", function(t)
   t:hideLandingSite("alphaCity")
   toOrbit(t)
   t:setLandingSiteShown("alphaCity", true)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectLandingSiteShown("alphaCity", false)
end)

test("landing sites: an unknown site name is a problem", function(t)
   toOrbit(t)
   t:allowProblem("has no landing site noSuchSite")
   t:setLandingSiteShown("noSuchSite", true)
end)

test("landing sites: outside the orbit scene is a problem", function(t)
   t:newGame()
   t:allowProblem("not in the orbit scene")
   t:setLandingSiteShown("goldMine", true)
end)
