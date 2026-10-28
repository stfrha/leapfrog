-- Swarms of strippers (lua/swarm.lua): started in a scene, stopped when
-- the Leapfrog has been stripped

local function setup(t)
   t:newGame()
   t:answer(t:mission(1).Question.acceptJob, true)
   t:clearDialog()
   local calls = {}
   local swarm = t:module("swarm")
   swarm.start("swarm", "stripper_swarm.xml", function(name) calls[#calls + 1] = name end)
   return swarm, calls
end

local function empty(t, keep)
   for _, id in ipairs({ 5, 6, 7, 8 }) do
      if id ~= keep then
         t:setProperty("leapfrog1", id, 0)
      end
   end
end

local function recalls(t)
   return t.engine:getObject("swarm").recalls
end

test("swarm: start adds the swarm and listens to the strippers' thefts", function(t)
   local swarm, calls = setup(t)
   t:expectProperty("swarm", 0, 1)          -- active
   t:expectProperty("swarm", 1, 0.2)        -- ramp starts at startRate
   t:expectProperty("swarm", 2, 2)          -- and startMaxAlive
   t:expectListening("leapfrog1", "LfSt", 1)
   t:expect(swarm.isRunning("swarm"), "swarm running")
end)

test("swarm: a theft while something is left changes nothing", function(t)
   local swarm, calls = setup(t)
   empty(t, 7)                              -- fuel left
   t:event("leapfrog1", "LfSt", 8)
   t:expectProperty("swarm", 0, 1)
   t:expectEqual(recalls(t), 0, "recalls")
   t:expectEqual(#calls, 0, "onStripped calls")
end)

test("swarm: stripped of everything: stops, recalls, tells the mission once", function(t)
   local swarm, calls = setup(t)
   empty(t)
   t:event("leapfrog1", "LfSt", 7)
   t:expectProperty("swarm", 0, 0)
   t:expectEqual(recalls(t), 1, "recalls")
   t:expectEqual(#calls, 1, "onStripped calls")
   t:expectEqual(calls[1], "swarm", "name given to onStripped")
   t:expect(not swarm.isRunning("swarm"), "swarm no longer running")
   t:event("leapfrog1", "LfSt", 0)
   t:expectEqual(#calls, 1, "onStripped calls after another LfSt")
end)

test("swarm: shield drained last (an empty-handed theft, parameter 0) also ends it", function(t)
   local swarm, calls = setup(t)
   empty(t)
   t:event("leapfrog1", "LfSt", 0)
   t:expectProperty("swarm", 0, 0)
   t:expectEqual(#calls, 1, "onStripped calls")
end)

test("swarm: with a resource list, only those count", function(t)
   t:newGame()
   t:answer(t:mission(1).Question.acceptJob, true)
   local swarm = t:module("swarm")
   local stripped = false
   swarm.start("swarm", "stripper_swarm.xml", function() stripped = true end, { 7 })
   t:setProperty("leapfrog1", 7, 0)         -- fuel gone, credits etc. left
   t:event("leapfrog1", "LfSt", 7)
   t:expect(stripped, "stripped when the fuel is gone")
end)

test("swarm: set changes rate and size; stop without recall leaves them be", function(t)
   local swarm, calls = setup(t)
   swarm.set("swarm", 1.5, 12)
   t:expectProperty("swarm", 1, 1.5)
   t:expectProperty("swarm", 2, 12)
   swarm.stop("swarm")
   t:expectProperty("swarm", 0, 0)
   t:expectEqual(recalls(t), 0, "recalls")
end)

test("swarm: a scene setup forgets the scene's swarms", function(t)
   local swarm, calls = setup(t)
   t:setupScene()
   t:expect(not swarm.isRunning("swarm"), "not running after the setup")
   empty(t)
   t:event("leapfrog1", "LfSt", 7)
   t:expectEqual(#calls, 0, "onStripped calls")
end)

test("swarm: started again at the next setup (a state change): kept, not added twice, still watched", function(t)
   local swarm, calls = setup(t)
   t:setProperty("swarm", 4, 3)             -- mark this swarm object
   t:setupScene()
   swarm.start("swarm", "stripper_swarm.xml", function(name) calls[#calls + 1] = name end)
   t:expectProperty("swarm", 4, 3)          -- the same object
   t:expectListening("leapfrog1", "LfSt", 1)
   empty(t)
   t:event("leapfrog1", "LfSt", 7)
   t:expectProperty("swarm", 0, 0)
   t:expectEqual(#calls, 1, "onStripped calls")
end)

test("swarm: a stopped swarm stays stopped when started again", function(t)
   local swarm, calls = setup(t)
   swarm.stop("swarm")
   t:setupScene()
   swarm.start("swarm", "stripper_swarm.xml", function(name) calls[#calls + 1] = name end)
   t:expectProperty("swarm", 0, 0)
   t:expect(not swarm.isRunning("swarm"), "not watched")
   empty(t)
   t:event("leapfrog1", "LfSt", 7)
   t:expectEqual(#calls, 0, "onStripped calls")
end)
