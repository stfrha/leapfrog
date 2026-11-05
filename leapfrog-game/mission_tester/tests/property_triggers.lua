-- Property triggers: each has an id of its own, OpTr says which one fired,
-- and one can be removed without touching the others

local credits = 8

local function start(t)
   t:newGame()
   local env = t.engine.env
   local a = env.c_registerPropertyTrigger("leapfrog1", credits, "insideRange", 0, 100)
   local b = env.c_registerPropertyTrigger("leapfrog1", credits, "insideRange", 0, 100)
   env.listen("leapfrog1", "OpTr")

   -- What arrives at the mission's handler, in order
   local fired = {}
   local handler = env.lua_missionStateSceneEventHandler
   env.lua_missionStateSceneEventHandler = function(eventId, actorName, parameter1)
      if eventId == "OpTr" then
         fired[#fired + 1] = parameter1
      end
      return handler(eventId, actorName, parameter1)
   end

   return env, a, b, fired
end

test("property triggers: each has its own id, and OpTr says which fired", function(t)
   local env, a, b, fired = start(t)
   t:expect(a > 0 and b > 0 and a ~= b, "two ids")
   t:setProperty("leapfrog1", credits, 50)
   t:expect(#fired == 2 and fired[1] == a and fired[2] == b, "both fired, each with its id")
end)

test("property triggers: removing one leaves the other on the same property", function(t)
   local env, a, b, fired = start(t)
   env.c_unregisterPropertyTrigger(a)
   t:setProperty("leapfrog1", credits, 50)
   t:expect(#fired == 1 and fired[1] == b, "only the other one fired")
end)

test("property triggers: an unknown id is ignored", function(t)
   local env, a, b, fired = start(t)
   env.c_unregisterPropertyTrigger(-1)
   env.c_unregisterPropertyTrigger(a + 1000)
   t:setProperty("leapfrog1", credits, 50)
   t:expect(#fired == 2, "both still fire")
end)

test("property triggers: an unknown object is a problem, and the id is -1", function(t)
   t:newGame()
   t:allowProblem("no object")
   t:expect(t.engine.env.c_registerPropertyTrigger("noSuchObject", 1, "insideRange", 0, 1) == -1, "-1")
end)
