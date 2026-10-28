-- The main menu's test row (lua/test_starts.lua): every test start must
-- start its scene, at its mission and state, with the Leapfrog in it, and
-- leave the saved game as it was

-- Checks test start i against its entry in lua/test_starts.lua
local function checkStart(t, i)
   local e = t:module("test_starts").list[i]
   t:menuTestStart(i)
   t:expectScene(e.scene)
   t:expectMission(e.mission)
   t:expectState(e.state)

   local lf = t.engine:getObject("leapfrog1")
   t:expect(lf ~= nil, "test start '" .. e.label .. "': no leapfrog1 in " .. e.scene)
   return e, lf
end

test("test starts: the menu lists every entry, in order", function(t)
   t:newGame()
   local labels = t:testStarts()
   local list = t:module("test_starts").list
   t:expectEqual(#labels, #list, "number of test starts")
   for i, e in ipairs(list) do
      t:expectEqual(labels[i], e.label, "label " .. i)
   end
end)

test("test starts: each one starts its scene with the Leapfrog in it", function(t)
   t:newGame()
   local list = t:module("test_starts").list
   for i = 1, #list do
      checkStart(t, i)
   end
end)

test("test starts: an arrival enters as after a real scene change", function(t)
   t:newGame()
   local list = t:module("test_starts").list
   for i, e in ipairs(list) do
      if e.from ~= nil then
         -- Compare with the navigator's own entry for this arrival
         t:setGlobal("currentScene", e.scene)
         t.engine:callLua("lua_findLeapfrogEntryPosition", e.from.type, e.from.param)
         local file = t:global("nextLfFile")
         local x, y = t:global("nextLfPosX"), t:global("nextLfPosY")
         local _, lf = checkStart(t, i)
         t:expectEqual(lf.file, file, e.label .. ": Leapfrog file")
         t:expectEqual(lf.x, x, e.label .. ": entry x")
         t:expectEqual(lf.y, y, e.label .. ": entry y")
      end
   end
end)

test("test starts: the saved game stays as it was", function(t)
   t:newGame()
   local S = t:mission(1).State
   t:expectSaved("scene", "home_scene.xml")
   local list = t:module("test_starts").list
   for i = 1, #list do
      t:menuTestStart(i)
      t:expectSaved("scene", "home_scene.xml")
      t:expectSaved("state", S.offered)
   end
end)

test("test starts: an arrival the navigator doesn't know is refused", function(t)
   t:newGame()
   local ts = t:module("test_starts")
   table.insert(ts.list, { label = "Home, from orbit", scene = "home_scene.xml", sceneType = 0,
      mission = 1, state = t:mission(1).State.accepted, from = ts.From.orbit })
   local ok, message = t.engine:menuTestStart(#ts.list)
   table.remove(ts.list)
   t:expect(not ok, "the start was refused")
   t:expect(tostring(message):find("no entry position", 1, true) ~= nil, "message: " .. tostring(message))
   t:expectScene("home_scene.xml")
end)

test("test starts: home with the job accepted shows the route, first pad flashing", function(t)
   t:newGame()
   local index
   for i, e in ipairs(t:module("test_starts").list) do
      if e.label == "Home, job accepted" then index = i end
   end
   t:expect(index ~= nil, "a 'Home, job accepted' test start")
   t:menuTestStart(index)
   t:expectMap("landingPad2", "siloHome", 2)     -- flashing slowly
   t:expectMap("landingPad3", "siloHome", 0)     -- hollow
end)
