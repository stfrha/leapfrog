-- Lua control of object factories (c_setFactoryProperty / c_getFactoryProperty)
-- against the mock, which follows ObjectFactory::setFactoryProperty: the
-- names are found in the scene, ids 0-6 mean the same, an inactive factory
-- waits with its spawnInitial, and spawn-now keeps within the limits.

local function inBigSpace(t)
   t:newGame()
   t:menuTestStart(2)
   t:expectScene("deep_space_scene.xml")
   return t.engine.env, t.engine.scene.factories["asteroidField"]
end

test("factory control: properties read what the scene file says", function(t)
   local env, f = inBigSpace(t)
   t:expect(f ~= nil, "asteroidField in the big space")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 0), 1, "active by default")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 1), f.intensity, "intensity")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), f.spawnInitial, "spawned = spawnInitial")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 6), 0, "spawn now reads 0")
end)

test("factory control: stop, change and restart a field", function(t)
   local env = inBigSpace(t)
   t:expect(env.c_setFactoryProperty("asteroidField", 0, 0), "deactivated")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 0), 0, "inactive")
   t:expect(env.c_setFactoryProperty("asteroidField", 1, 2.5), "intensity set")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 1), 2.5, "intensity")
   t:expect(env.c_setFactoryProperty("asteroidField", 0, 1), "activated")
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 0), 1, "active again")
end)

test("factory control: spawn now keeps within spawnTotal, and a new round starts from 0", function(t)
   local env = inBigSpace(t)
   local before = env.c_getFactoryProperty("asteroidField", 5)
   env.c_setFactoryProperty("asteroidField", 3, before + 3)    -- spawnTotal
   env.c_setFactoryProperty("asteroidField", 6, 10)            -- wants 10, gets 3
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), before + 3, "spawned")
   env.c_setFactoryProperty("asteroidField", 5, 0)             -- new round
   env.c_setFactoryProperty("asteroidField", 6, 2)
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), 2, "spawned in the new round")
end)

test("factory control: an inactive factory waits with its initial spawn", function(t)
   local env, f = inBigSpace(t)
   -- As if the scene file said active="false"
   f.active, f.spawned, f.initialPending = false, 0, true
   f.spawnTotal, f.maxAlive = 0, 0
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), 0, "nothing yet")
   env.c_setFactoryProperty("asteroidField", 0, 1)
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), f.spawnInitial, "initial spawn on activation")
   env.c_setFactoryProperty("asteroidField", 0, 0)
   env.c_setFactoryProperty("asteroidField", 0, 1)
   t:expectEqual(env.c_getFactoryProperty("asteroidField", 5), f.spawnInitial, "only once")
end)

test("factory control: alive is read only, and an unknown factory is a problem", function(t)
   local env = inBigSpace(t)
   t:allowProblem("property 4 can't be set")
   t:allowProblem("no object factory 'noSuchFactory'")
   t:expect(not env.c_setFactoryProperty("asteroidField", 4, 3), "alive can't be set")
   t:expect(not env.c_setFactoryProperty("noSuchFactory", 0, 0), "no such factory")
   t:expectEqual(env.c_getFactoryProperty("noSuchFactory", 0), 0, "reads 0")
end)
