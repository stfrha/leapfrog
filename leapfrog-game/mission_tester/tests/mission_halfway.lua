-- The mission "halfway" (lua/mission_halfway.lua): take the station back.
-- Bruno calls when the stash mission ends at the silo. Up there the
-- station's shield is red and its power unit sits on the boom; coming close
-- brings the strippers. Shoot the unit off, dock and hold on while the hack
-- runs; then the station is his, its master hands over the bad mine's
-- solution, and a while later the Baxters pull out.

local station = "dockingStation1"
local power = "shieldPower1"
local shieldProp = 5

local function offered(t)
   t:newGame()
   t:land("siloLandingPad")                 -- where stash ends
   t.engine:callLua("lua_startMission", "halfway")
   t.engine:updateQuestions()               -- (the game does this every frame)
   t:expectMission("halfway")
   return t:mission("halfway")
end

local function inSpace(t)
   local m = offered(t)
   t:answer(m.Question.takeBack, true)
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   return m
end

local function close(t, m)
   t:setProperty("leapfrog1", 3, m.approachX + 50)
end

local function powerDown(t)
   local m = inSpace(t)
   close(t, m)
   t:breakObject(power)
   t:expectState(m.State.powerDown)
   return m
end

local function hacked(t)
   local m = powerDown(t)
   t:event(station, "DsDk")
   t:wait(m.hackTicks / 60 + 1)
   t:expectState(m.State.liberated)
   return m
end

local function swarmActive(t)
   local s = t.engine:getObject("swarm")
   return s ~= nil and s.props[0] == 1
end

local function strip(t)
   for _, id in ipairs({ 5, 6, 7, 8 }) do
      t:setProperty("leapfrog1", id, 0)
   end
   t:event("leapfrog1", "LfSt", 7)
end

test("halfway: stash ends at the silo, and Bruno calls about the station", function(t)
   local m = offered(t)
   t:expectState(m.State.offered)
   t:expectDialog("The Baxters have taken the station.", "Bruno")
   t:expectQuestion(m.Question.takeBack)
end)

test("halfway: taking it shows the launch site, flashing", function(t)
   local m = offered(t)
   t:answer(m.Question.takeBack, true)
   t:expectState(m.State.toLaunch)
   t:expectMapShown("launchSite1", "mainTank", true)
   t:expectMap("launchSite1", "mainTank", "flashingSlow")
end)

test("halfway: the station is held: red shield, the power unit, their ship; no hammers", function(t)
   local m = inSpace(t)
   t:expectState(m.State.toStation)
   t:expectProperty(station, shieldProp, m.Shield.hostile)
   t:expect(t.engine:getObject(power) ~= nil, "the power unit is on the boom")
   t:expectMap(station, "stationBase", "flashingSlow")
   t:expect(t.engine.scene.moves.baxterShip ~= nil, "their homestead is parked there")
   t:expect(t.engine:getObject("hammer1") == nil, "no hammers")
   t:expect(t.engine:getObject("interceptor1") == nil, "no interceptors")
   t:expect(not swarmActive(t), "no strippers yet")
end)

test("halfway: coming close brings the strippers", function(t)
   local m = inSpace(t)
   t:setProperty("leapfrog1", 3, m.approachX - 100)
   t:expect(not swarmActive(t), "not from far away")
   close(t, m)
   t:expect(swarmActive(t), "the strippers come")
end)

test("halfway: the power unit shot off: the shield drops, the strippers come", function(t)
   local m = inSpace(t)
   t:breakObject(power)
   t:expectState(m.State.powerDown)
   t:expectProperty(station, shieldProp, m.Shield.off)
   t:expectHud("Station shield down")
   t:expect(swarmActive(t), "the strippers come (at the latest now)")
end)

test("halfway: docking with the shield up does nothing", function(t)
   local m = inSpace(t)
   t:event(station, "DsDk")
   t:expectState(m.State.toStation)
end)

test("halfway: docked, the hack runs in steps", function(t)
   local m = powerDown(t)
   t:event(station, "DsDk")
   t:expectState(m.State.hacking)
   t:expectHud("Hacking: 0%")
   t:wait(m.hackTicks / 60 / 2 + 0.5)
   t:expectHud("Hacking: 50%")
   t:expectState(m.State.hacking)
end)

test("halfway: undocking loses the hack; docking again starts it over", function(t)
   local m = powerDown(t)
   t:event(station, "DsDk")
   t:wait(m.hackTicks / 60 / 2)
   t:event(station, "DsUd")
   t:expectState(m.State.powerDown)
   t:expectHud("Hack lost")
   t:wait(m.hackTicks / 60)
   t:expectState(m.State.powerDown)
   t:event(station, "DsDk")
   t:expectState(m.State.hacking)
   t:wait(m.hackTicks / 60 + 1)
   t:expectState(m.State.liberated)
end)

test("halfway: hacked: friendly shield, strippers called back, the bad mine from the station master", function(t)
   local m = hacked(t)
   t:expectProperty(station, shieldProp, m.Shield.friendly)
   t:expectHud("Halfway is yours")
   t:expect(not swarmActive(t), "the swarm has stopped")
   t:expectEqual(t.engine:getObject("swarm").recalls, 1, "strippers recalled")
   local master = t:module("mining").stationMaster
   t:expectDialog("Welcome back to Halfway", master)
   t:expectDialog("The Baxters left a few things lying about.", master)
   t:expectDialog("I work for you now.", master)
   t:expectItem("badMineSolution", "held")
   t:expectMap(station, "stationBase", "hollow")
end)

test("halfway: a while after the hack the Baxters pull out; free play", function(t)
   local m = hacked(t)
   t:wait(m.pullOutTicks / 60 - 2)
   t:expectState(m.State.liberated)
   t:wait(3)
   t:expectDialog("Father Baxter's warning", "Father Baxter")
   t:expectEqual(t.engine.scene.moves.baxterShip.alpha, 0, "their homestead leaves")
   t:expectHud("Mission complete")
   t:expectMission("freePlay")
   t:expect(not t:module("mining").baxtersInSpace(), "the Baxters are gone from space")
end)

test("halfway: stripped bare: fuel to limp home, try again from the launch site", function(t)
   local m = powerDown(t)
   t:setProperty("leapfrog1", 7, 0)
   strip(t)
   t:expectState(m.State.beaten)
   t:expectProperty("leapfrog1", 7, m.limpHomeFuel)
   t:expectDialog("Go home, little frog.", "Raiders")
   t:exitScene(1, 3)
   t:reentry("onSite", 1)
   t:expectScene("home_scene.xml")
   t:expectMap("launchSite1", "mainTank", "flashingSlow")
   t:land("siloLandingPad")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectState(m.State.toStation)
   t:expectProperty(station, shieldProp, m.Shield.hostile)
   t:expect(t.engine:getObject(power) ~= nil, "a new power unit")
end)

test("halfway: leaving with the shield down and coming back: still down, the hack starts over", function(t)
   local m = powerDown(t)
   t:event(station, "DsDk")
   t:exitScene(1, 3)
   t:restartGame()
   t:expectScene("orbit_scene.xml")
   t:exitScene(2, -3)                       -- bounced: back in space
   t:expectScene("deep_space_scene.xml")
   t:expectState(m.State.powerDown)
   t:expectProperty(station, shieldProp, m.Shield.off)
   t:expect(t.engine:getObject(power) == nil, "no power unit")
end)

test("halfway: back in space after the hack, before they pull out: they still do", function(t)
   local m = hacked(t)
   t:exitScene(1, 3)
   t:exitScene(2, -3)
   t:expectScene("deep_space_scene.xml")
   t:expectState(m.State.liberated)
   t:expectProperty(station, shieldProp, m.Shield.friendly)
   t:wait(m.pullOutTicks / 60 + 1)
   t:expectMission("freePlay")
end)

test("halfway: the pull-out lines are placeholders for Fredrik", function(t)
   local m = offered(t)
   t:expect(#m.pullOutLines >= 1, "some lines")
   for _, line in ipairs(m.pullOutLines) do
      t:expect(type(line[1]) == "string" and type(line[2]) == "string", "sender and text")
   end
end)
