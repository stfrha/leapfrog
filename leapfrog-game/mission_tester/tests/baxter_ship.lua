-- The Baxters' homestead ship in the background (lua/baxter_ship.lua):
-- hidden at the start, arrives, parks with its engines cold, leaves

local function moves(t)
   return t.engine.scene.moves.baxterShip
end

local function frame(t)
   local a = t.engine.scene.animations.baxterShip
   return a and a.frame
end

local function freePlayInSpace(t)
   t:newGame()
   t.engine:callLua("lua_startMission", "freePlay")
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
end

test("baxter ship: in the scene file, hidden at the start, in the scene behind everything", function(t)
   local root = t.engine:readXml("deep_space_scene.xml")
   local ship
   for _, c in ipairs(root.children) do
      if c.tag == "spriteBox" and c.attr.name == "baxterShip" then
         ship = c
      end
   end
   t:expect(ship ~= nil, "a spriteBox baxterShip")
   t:expectEqual(tonumber(ship.attr.alpha), 0, "alpha")
   -- Not on a parallax layer: there it followed the Leapfrog about
   t:expectEqual(tonumber(ship.attr.parallaxAmount or 0), 0, "in the scene, not on a parallax layer")
   t:expectEqual(tonumber(ship.attr.zLevel), 0, "behind everything")
   t:expectEqual(ship.attr.texture, "baxter_homestead", "texture")
end)

test("baxter ship: free play in deep space: comes by, parks cold, leaves", function(t)
   freePlayInSpace(t)
   local B = t:module("baxterShip")
   t:expect(moves(t) == nil, "not moved before it arrives")
   t:wait(16)
   local m = moves(t)
   t:expect(m ~= nil, "it arrives")
   t:expectEqual(m.x, B.park.x, "parks at x")
   t:expectEqual(m.alpha, B.haze, "visible, hazy")
   t:expectEqual(frame(t), 0, "engines burning on the way in")
   t:wait(B.arriveSeconds + 4)
   t:expectEqual(frame(t), 1, "engines cold when parked")
   t:wait(60)
   m = moves(t)
   t:expectEqual(m.alpha, 0, "fades out leaving")
   t:expectEqual(frame(t), 0, "engines burning on the way out")
end)

test("baxter ship: not in a mission's space unless the mission brings it", function(t)
   t:newGame()                              -- mission 1
   t:exitScene(0, 1)                        -- up into deep space
   t:expectScene("deep_space_scene.xml")
   t:wait(30)
   t:expect(moves(t) == nil, "no ship")
end)

test("baxter ship: show and hide at once", function(t)
   freePlayInSpace(t)
   local B = t:module("baxterShip")
   B.show()
   t:expectEqual(moves(t).alpha, B.haze, "shown")
   t:expectEqual(moves(t).seconds, 0, "at once")
   t:expectEqual(frame(t), 1, "parked with cold engines")
   B.hide()
   t:expectEqual(moves(t).alpha, 0, "hidden")
end)

local function tagMove(t, name)
   return t.engine.scene.moves[name or "baxterTag"]
end

test("baxter ship: the tag is in the scene file, hidden, on the ship's layer", function(t)
   local root = t.engine:readXml("deep_space_scene.xml")
   local ship, tag
   for _, c in ipairs(root.children) do
      if c.tag == "spriteBox" and c.attr.name == "baxterShip" then ship = c end
      if c.tag == "spriteBox" and c.attr.name == "baxterTag" then tag = c end
   end
   t:expect(tag ~= nil, "a spriteBox baxterTag")
   t:expectEqual(tonumber(tag.attr.alpha), 0, "alpha")
   t:expectEqual(tag.attr.parallaxAmount, ship.attr.parallaxAmount, "same layer as the ship")
   t:expect(tonumber(tag.attr.zLevel) > tonumber(ship.attr.zLevel), "above the ship")
end)

test("baxter ship: the tag follows the ship, at its offset times the scale", function(t)
   freePlayInSpace(t)
   local B = t:module("baxterShip")
   local o = B.overlays.baxterShip.baxterTag
   local size = B.places["deep_space_scene.xml"].size
   B.arrive()
   local s, g = moves(t), tagMove(t)
   t:expectEqual(g.x, s.x + o.dx * s.scale * size, "tag x")
   t:expectEqual(g.y, s.y + o.dy * s.scale * size, "tag y")
   t:expectEqual(g.scale, s.scale, "tag scale")
   t:expectEqual(g.alpha, B.haze, "tag shows with the ship")
   t:expectEqual(g.seconds, s.seconds, "same time")
   t:expectEqual(g.ease, s.ease, "same ease")
   B.leave()
   s, g = moves(t), tagMove(t)
   t:expectEqual(g.x, s.x + o.dx * s.scale * size, "tag x leaving")
   t:expectEqual(g.alpha, 0, "tag fades with the ship")
   B.hide()
   t:expectEqual(tagMove(t).alpha, 0, "hidden with the ship")
end)

test("baxter ship: tag(nil) leaves the doors bare from the next show", function(t)
   freePlayInSpace(t)
   local B = t:module("baxterShip")
   B.tag(nil)
   B.show()
   t:expectEqual(moves(t).alpha, B.haze, "ship shown")
   t:expectEqual(tagMove(t).alpha, 0, "no tag")
   B.tag("baxterTag")
   B.show()
   t:expectEqual(tagMove(t).alpha, B.haze, "tag back")
   t:expectEqual(B.shownTag(), "baxterTag", "shownTag")
end)

-- The sprites in each scene file are the size its place says, the tag where
-- the script puts it, and both the same hazy colour
test("baxter ship: each scene's sprites match its place: size, tag offset, colour", function(t)
   t:newGame()
   local B = t:module("baxterShip")
   local o = B.overlays.baxterShip.baxterTag
   for scene, p in pairs(B.places) do
      local ship, tag
      for _, c in ipairs(t.engine:readXml(scene).children) do
         if c.tag == "spriteBox" and c.attr.name == "baxterShip" then ship = c end
         if c.tag == "spriteBox" and c.attr.name == "baxterTag" then tag = c end
      end
      t:expect(ship and tag, scene .. ": ship and tag")
      t:expect(math.abs(tonumber(ship.attr.width) - 480 * p.size) < 0.01, scene .. ": ship width")
      t:expect(math.abs(tonumber(ship.attr.posX) - p.x) < 0.01, scene .. ": ship x")
      t:expect(math.abs(tonumber(tag.attr.posX) - (p.x + o.dx * p.size)) < 0.06, scene .. ": tag x")
      t:expect(math.abs(tonumber(tag.attr.posY) - (p.y + o.dy * p.size)) < 0.06, scene .. ": tag y")
      t:expectEqual(tag.attr.color, ship.attr.color, scene .. ": same colour")
      t:expectEqual(tonumber(tag.attr.parallaxAmount or 0), 0, scene .. ": tag in the scene")
   end
end)
