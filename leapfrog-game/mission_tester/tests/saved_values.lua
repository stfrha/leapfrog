-- Saved values (c_setSavedValue): numbers kept from scene to scene, saved
-- like the key items

local function start(t)
   t:newGame()
   return t.engine.env
end

test("saved values: a value not set is the default, or 0", function(t)
   local env = start(t)
   t:expectEqual(env.c_getSavedValue("nothing"), 0, "no default")
   t:expectEqual(env.c_getSavedValue("nothing", 7), 7, "default")
end)

test("saved values: kept when saved, gone when the game ends first", function(t)
   local env = start(t)
   env.c_setSavedValue("kept", 3)
   t:expectEqual(env.c_getSavedValue("kept"), 3, "at once")
   t:restartGame()
   t:expectEqual(t.engine.env.c_getSavedValue("kept"), 0, "not saved yet")

   t.engine.env.c_setSavedValue("kept", 4)
   t.engine:saveGameStatus()
   t:restartGame()
   t:expectEqual(t.engine.env.c_getSavedValue("kept"), 4, "saved")
end)

test("saved values: a new game starts over", function(t)
   local env = start(t)
   env.c_setSavedValue("kept", 3)
   t.engine:saveGameStatus()
   t:newGame()
   t:expectEqual(t.engine.env.c_getSavedValue("kept"), 0, "cleared")
end)

test("baxters: not about in a new game; a mission's say is saved", function(t)
   t:newGame()
   local mining = t:module("mining")
   t:expect(not mining.baxtersInSpace(), "not about")
   mining.setBaxtersInSpace(true)
   t.engine:saveGameStatus()
   t:restartGame()
   t:expect(t:module("mining").baxtersInSpace(), "about after a continue")
   t:module("mining").setBaxtersInSpace(false)
   t:expect(not t:module("mining").baxtersInSpace(), "gone")
end)
