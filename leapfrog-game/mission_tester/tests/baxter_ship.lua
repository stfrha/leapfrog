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
   t.engine:callLua("lua_startMission", 0)
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
end

test("baxter ship: in the scene file, hidden at the start, on a far layer", function(t)
   local root = t.engine:readXml("deep_space_scene.xml")
   local ship
   for _, c in ipairs(root.children) do
      if c.tag == "spriteBox" and c.attr.name == "baxterShip" then
         ship = c
      end
   end
   t:expect(ship ~= nil, "a spriteBox baxterShip")
   t:expectEqual(tonumber(ship.attr.alpha), 0, "alpha")
   t:expect(tonumber(ship.attr.parallaxAmount) >= 0.8, "on a far parallax layer")
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
   t:expectEqual(m.alpha, 1, "visible")
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
   t:expectEqual(moves(t).alpha, 1, "shown")
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
   B.arrive()
   local s, g = moves(t), tagMove(t)
   t:expectEqual(g.x, s.x + o.dx * s.scale, "tag x")
   t:expectEqual(g.y, s.y + o.dy * s.scale, "tag y")
   t:expectEqual(g.scale, s.scale, "tag scale")
   t:expectEqual(g.alpha, 1, "tag shows with the ship")
   t:expectEqual(g.seconds, s.seconds, "same time")
   t:expectEqual(g.ease, s.ease, "same ease")
   B.leave()
   s, g = moves(t), tagMove(t)
   t:expectEqual(g.x, s.x + o.dx * s.scale, "tag x leaving")
   t:expectEqual(g.alpha, 0, "tag fades with the ship")
   B.hide()
   t:expectEqual(tagMove(t).alpha, 0, "hidden with the ship")
end)

test("baxter ship: tag(nil) leaves the doors bare from the next show", function(t)
   freePlayInSpace(t)
   local B = t:module("baxterShip")
   B.tag(nil)
   B.show()
   t:expectEqual(moves(t).alpha, 1, "ship shown")
   t:expectEqual(tagMove(t).alpha, 0, "no tag")
   B.tag("baxterTag")
   B.show()
   t:expectEqual(tagMove(t).alpha, 1, "tag back")
   t:expectEqual(B.shownTag(), "baxterTag", "shownTag")
end)
