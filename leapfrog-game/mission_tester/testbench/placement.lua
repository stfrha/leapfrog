-- Where things may be placed in a scene. The soft boundaries don't start at
-- the scene's edge: their push starts half a boundary width inside it, and
-- grows with the square of the depth. Something bounded that starts deep in
-- a boundary is thrown across the scene. See src/softboundary.cpp.

local xml = require("testbench.xml")

local placement = {}

-- The boundary width, from SceneActor::defineScene
local function boundaryWidth(srcDir)
   local f = io.open(srcDir .. "/sceneactor.cpp", "rb")
   if f == nil then
      return nil
   end
   local text = f:read("a")
   f:close()
   return tonumber(text:match("initStageSize%(Vector2%(width, height%), ([%d%.]+)f%)"))
end

-- A scene's size and boundaries from its XML root, nil for a scene without
-- sceneProperties (the orbit scene has its own coordinates)
function placement.sceneBox(root, srcDir)
   local behaviour = xml.child(root, "behaviour")
   local props = behaviour and xml.child(behaviour, "sceneProperties")
   if props == nil then
      return nil
   end

   local width = boundaryWidth(srcDir)
   if width == nil then
      error("the boundary width isn't in sceneactor.cpp any more: update testbench/placement.lua")
   end

   local a = props.attr
   local inset = width / 2
   return {
      space = a.sceneType == "space",
      width = tonumber(a.width) or 1000,
      height = tonumber(a.height) or 500,
      inset = inset,
      left = a.hasLeftBoundary == "true" and inset or nil,
      right = a.hasRightBoundary == "true" and inset or nil,
      top = a.hasUpperBoundary == "true" and inset or nil,
      bottom = a.hasLowerBoundary == "true" and inset or nil,
   }
end

-- How deep in a boundary a point is, 0 outside the push
function placement.depth(box, x, y)
   local d = 0
   if box.left then d = math.max(d, box.left - x) end
   if box.right then d = math.max(d, x - (box.width - box.right)) end
   if box.top then d = math.max(d, box.top - y) end
   if box.bottom then d = math.max(d, y - (box.height - box.bottom)) end
   return d
end

function placement.insideScene(box, x, y)
   return x >= 0 and x <= box.width and y >= 0 and y <= box.height
end

-- The speed a body of this mass gets from starting at this depth: the push
-- is 50 * depth^2, so the work is 50 * depth^3 / 3 = mass * speed^2 / 2
function placement.kickSpeed(depth, mass)
   return math.sqrt(100.0 * depth ^ 3 / (3.0 * mass))
end

-- What is wrong with an object at (x, y), nil if nothing: in a space scene
-- objects are pushed by the boundaries, so they must be clear of them; in a
-- ground scene they only need to be in the scene
function placement.objectProblem(box, x, y, allowedDepth)
   if box.space then
      local depth = placement.depth(box, x, y)
      if depth > (allowedDepth or 0) then
         return string.format("(%g, %g) is %g deep in the boundary of a %g x %g scene",
            x, y, depth, box.width, box.height)
      end
   elseif not placement.insideScene(box, x, y) then
      return string.format("(%g, %g) is outside the %g x %g scene", x, y, box.width, box.height)
   end
   return nil
end

return placement
