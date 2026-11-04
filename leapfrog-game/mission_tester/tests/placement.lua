-- Nothing starts where the boundaries would throw it: the object factories'
-- areas, the objects in the scene files, the Leapfrog's entries and what the
-- scripts add. The mock engine also checks every c_addPositionedChildObject
-- while the other tests play (Engine.placedOutside lists the exceptions).

local harness = require("testbench.harness")
local placement = require("testbench.placement")
local xml = require("testbench.xml")

local function sceneRoot(file)
   return xml.readFile(harness.dataDir .. "/" .. file)
end

-- Every scene the scripts know, with its box
local function scenes(t)
   local names = {}
   for name, _ in pairs(t:global("sceneWorld")) do names[name] = true end
   for name, _ in pairs(t:global("spaces")) do names[name] = true end
   for name, _ in pairs(t:global("orbitArrivals")) do names[name] = true end

   local list = {}
   for name, _ in pairs(names) do
      local root = sceneRoot(name)
      t:expect(root ~= nil, "can't read " .. name)
      local box = placement.sceneBox(root, harness.srcDir)
      if box ~= nil then
         list[#list + 1] = { name = name, root = root, box = box }
      end
   end
   table.sort(list, function(a, b) return a.name < b.name end)
   return list
end

-- Fails the test with every problem found, so one run lists them all
local function report(t, problems)
   t:expect(#problems == 0, "\n         " .. table.concat(problems, "\n         "))
end

test("placement: object factories spawn clear of the boundaries", function(t)
   t:newGame()
   local problems = {}
   for _, scene in ipairs(scenes(t)) do
      for _, c in ipairs(scene.root.children) do
         local p = c.tag == "system" and c.attr.type == "objectFactory" and
            xml.child(c, "objectFactoryProperties")
         if p and (p.attr.body == nil or p.attr.body == "notApplicable") then
            local x, y = tonumber(p.attr.posX), tonumber(p.attr.posY)
            local w, h = tonumber(p.attr.width), tonumber(p.attr.height)
            for _, corner in ipairs({ { x, y }, { x + w, y }, { x, y + h }, { x + w, y + h } }) do
               local problem = placement.objectProblem(scene.box, corner[1], corner[2])
               if problem then
                  problems[#problems + 1] = scene.name .. " factory " .. tostring(c.attr.name) ..
                     ": corner " .. problem
               end
            end
         end
      end
   end
   report(t, problems)
end)

test("placement: the objects in the scene files are clear of the boundaries", function(t)
   t:newGame()
   local problems = {}
   for _, scene in ipairs(scenes(t)) do
      for _, c in ipairs(scene.root.children) do
         if c.tag == "childObject" then
            local problem = placement.objectProblem(scene.box, tonumber(c.attr.posX) or 0, tonumber(c.attr.posY) or 0)
            if problem then
               problems[#problems + 1] = scene.name .. " object " .. tostring(c.attr.name) .. ": " .. problem
            end
         end
      end
   end
   report(t, problems)
end)

test("placement: the Leapfrog enters inside the scene", function(t)
   t:newGame()
   local problems = {}

   local function check(sceneName, what, entry)
      local box = placement.sceneBox(sceneRoot(sceneName), harness.srcDir)
      if not placement.insideScene(box, entry.x, entry.y) then
         problems[#problems + 1] = string.format("%s %s: (%g, %g) is outside the %g x %g scene",
            sceneName, what, entry.x, entry.y, box.width, box.height)
      end
   end

   for name, space in pairs(t:global("spaces")) do
      check(name, "launch", space.launch)
      check(name, "bounce", space.bounce)
   end
   for name, arrival in pairs(t:global("orbitArrivals")) do
      for pad, entry in pairs(arrival.pads) do
         check(name, pad .. " from orbit", entry.from)
         check(name, pad .. " on the pad", entry.onPad)
      end
   end
   report(t, problems)
end)

test("placement: after a bounce the Leapfrog is out of the exit's transit field", function(t)
   t:newGame()
   local problems = {}
   for name, space in pairs(t:global("spaces")) do
      local box = placement.sceneBox(sceneRoot(name), harness.srcDir)
      local lowerRight = space.bounce.x > box.width - box.right and space.bounce.y > box.height - box.bottom
      if lowerRight then
         problems[#problems + 1] = name .. ": the bounce entry is in the lower right transit field"
      end
   end
   report(t, problems)
end)

test("placement: every test start begins with everything in place", function(t)
   t:newGame()
   for i = 1, #t:testStarts() do
      t:menuTestStart(i)
   end
end)

test("placement: the stash asteroid drifts in, the boundary doesn't throw it", function(t)
   t:newGame()
   local stash = t:mission(2).Stash
   local box = placement.sceneBox(sceneRoot(stash.space), harness.srcDir)

   t:expect(placement.depth(box, stash.x, stash.y) > 0, "it comes from the boundary, not from inside")

   local root = sceneRoot("stash_asteroid.xml")
   local body = xml.child(root, "dynamicCircle")
   local mass = tonumber(body.attr.density) * math.pi * tonumber(body.attr.radius) ^ 2
   local kick = placement.kickSpeed(placement.depth(box, stash.x, stash.y), mass)
   t:expect(kick <= 50, string.format("the boundary would throw it at %.0f units/s", kick))
end)

test("placement: a swarm has room to spawn away from the Leapfrog", function(t)
   t:newGame()
   local root = sceneRoot("stripper_swarm.xml")
   local props = xml.child(xml.child(root, "behaviour"), "swarmProperties")
   local minDistance = tonumber(props.attr.minDistance)

   for _, scene in ipairs(scenes(t)) do
      if scene.box.space then
         local fromCentre = math.sqrt(scene.box.width ^ 2 + scene.box.height ^ 2) / 2
         t:expect(fromCentre >= minDistance, scene.name .. " is too small for the swarm's minDistance")
      end
   end
end)
