-- Test starts: the main menu's test row (< label >) steps through this
-- list; tapping the label starts that entry at once, the way the game
-- would get there: the scene, the mission and its state, and where the
-- Leapfrog arrives from (so it enters as after a real scene change).
--
-- To add one, add a line to the list. Fields:
--   label      shown in the menu (keep it short, about 25 characters)
--   scene      scene file
--   sceneType  0 landing, 1 deep space, 2 orbit
--   mission    mission number (0 = free play)
--   state      the mission's state, by name (State.accepted)
--   from       where the Leapfrog arrives from (From.orbit, ...), or nil to
--              start at the scene's own initial position
--
-- A "from" must be an arrival scene_navigator.lua knows
-- (lua_findLeapfrogEntryPosition): otherwise the start is refused with a
-- message, and the mission tester (tests/test_starts.lua) fails on it.
--
-- A test start doesn't save the game when it starts (the saved game stays
-- as it was); scene exits after it save as usual.

local mission1 = require('mission1')
local S = mission1.State

local testStarts = {}

-- Arrivals: the exit of the scene the Leapfrog comes from, as
-- lua_sceneExitHandler gets it (exit scene, exit type, exit parameter)
local From = {
   orbit  = { scene = "orbit_scene.xml",      type = 2, param = 0 },  -- landed from orbit (site 0)
   launch = { scene = "landing_scene.xml",    type = 0, param = 1 },  -- launched into deep space
   space  = { scene = "deep_space_scene.xml", type = 1, param = 3 },  -- left deep space, lower right, to orbit
}
testStarts.From = From

testStarts.list = {
   { label = "Landing, from orbit",  scene = "landing_scene.xml",    sceneType = 0, mission = 1, state = S.testLandOnPad, from = From.orbit },
   { label = "Space, launched",      scene = "deep_space_scene.xml", sceneType = 1, mission = 1, state = S.testLandOnPad, from = From.launch },
   { label = "Orbit, from space",    scene = "orbit_scene.xml",      sceneType = 2, mission = 1, state = S.testLandOnPad, from = From.space },
   { label = "Home, job accepted",   scene = "home_scene.xml",       sceneType = 0, mission = 1, state = S.accepted },
}

return testStarts
