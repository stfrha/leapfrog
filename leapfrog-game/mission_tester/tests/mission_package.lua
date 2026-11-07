-- The mission "package" (lua/mission_package.lua): after the beacons, a calm
-- first trip to the station. Rocks, docking, the station master, a sealed
-- package for Bruno, delivered on the village pad; then the stash is offered.

local station = "dockingStation1"
local enemies = { "hammer1", "interceptor1", "interceptor2", "mineLayer", "mine1" }

local function offered(t)
   t:throughBeacons()
   return t:mission("package")
end

local function inSpace(t)
   local m = offered(t)
   t:answer(m.Question.fetch, true)
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   return m
end

local function docked(t)
   local m = inSpace(t)
   t:event(station, "DsDk")
   return m
end

local function home(t)
   local m = docked(t)
   t:exitScene(1, 3)
   t:reentry("onSite", 1)
   t:expectScene("home_scene.xml")
   return m
end

test("package: offered on the village pad after the beacons", function(t)
   local m = offered(t)
   t:expectMission("package")
   t:expectState(m.State.offered)
   t:expectDialog("a package waiting for me up at the station", "Bruno")
   t:expectQuestion(m.Question.fetch)
end)

test("package: taking it shows the launch site; the launch goes to the big space", function(t)
   local m = offered(t)
   t:answer(m.Question.fetch, true)
   t:expectState(m.State.toLaunch)
   t:expectMapShown("launchSite1", "mainTank", true)
   t:expectMap("launchSite1", "mainTank", "flashingSlow")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectState(m.State.toStation)
   -- Her first trip to space
   t:expectDialog("Yay! We are going to space!", "Edith")
   t:expectDialog("How old are you?", "Leapfrog")
   t:expectDialog("Brace for launch now!", "Edith")
end)

test("package: going up without answering takes the job", function(t)
   local m = offered(t)
   t:answer(m.Question.fetch, false)
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectState(m.State.toStation)
end)

test("package: space is calm: no enemies, no strippers, the station as it first is", function(t)
   inSpace(t)
   for _, name in ipairs(enemies) do
      t:expect(t.engine:getObject(name) == nil, name .. " is taken out")
   end
   t:expect(t.engine:getObject("asteroid1") ~= nil, "the rocks stay")
   t:expect(t.engine:getObject("shieldPower1") == nil, "no shield power unit")
   t:expect(t.engine:getObject("swarm") == nil, "no swarm")
   t:expectListening(station, "DsDk", 1)
end)

test("package: Edith talks him in; the station flashes on the map", function(t)
   inSpace(t)
   t:expectDialog("Come in slow, nose first, right under the arm", "Edith")
   t:expectMap(station, "stationBase", "flashingSlow")
end)

test("package: Edith reminds him if he hasn't docked, not after", function(t)
   inSpace(t)
   t:wait(46)
   t:expectDialog("The station is the one flashing on your map", "Edith")
end)

test("package: no reminder once docked", function(t)
   docked(t)
   t:wait(46)
   t:expectNoDialog("The station is the one flashing on your map")
end)

test("package: docking: the station master's welcome, fuel, ammo, credits, the package", function(t)
   local m = inSpace(t)
   t:setProperty("leapfrog1", 7, 10)
   local ammo = t:property("leapfrog1", 5)
   local credits = t:property("leapfrog1", 8)
   t:event(station, "DsDk")
   local master = t:module("mining").stationMaster
   t:expectDialog("Welcome to Halfway, friend!", master)
   t:expectDialog("It's everybody's station.", master)
   t:expectDialog("Sealed, as he asked.", master)
   t:expectProperty("leapfrog1", 7, m.welcomeFuel)
   t:expectProperty("leapfrog1", 5, ammo + m.welcomeAmmo)
   t:expectProperty("leapfrog1", 8, credits + m.rockMoney)
   t:expectState(m.State.carrying)
   t:expectMap(station, "stationBase", "hollow")
   t:expectHud("Package aboard")
   t:expectDialog("just boost off", master)
end)

test("package: the station keeps its secrets for later: no bad mine solution yet", function(t)
   docked(t)
   t:expectItem("badMineSolution", "unknown")
end)

test("package: docking again on the trip says nothing more", function(t)
   docked(t)
   t:event(station, "DsDk")
   t:expectEqual(t:countDialog("Welcome to Halfway, friend!"), 1, "welcomes")
end)

test("package: on the way down, Edith wonders what's in it", function(t)
   docked(t)
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:expectDialog("What do you think is in it?", "Edith")
end)

test("package: the re-entry ends at the village pad; the Ore is the wrong place", function(t)
   local m = home(t)
   t:expectState(m.State.carrying)
   t:expectMap("villagePad", "siloHome", "flashingSlow")
   t:expectMap("launchSite1", "mainTank", "hollow")    -- not where he's going
   t:land("oreLanding")
   t:expectDialog("Not here. The pad outside the village.", "Bruno")
   t:expectMission("package")
end)

test("package: delivered on the village pad: Bruno pays, and the stash is offered", function(t)
   local m = home(t)
   local credits = t:property("leapfrog1", 8)
   t:land("villagePad")
   t:expectDialog("You didn't shake it, I hope.", "Bruno")
   t:expectProperty("leapfrog1", 8, credits + m.deliveryPay)
   t:expectMission("stash")
   t:expectState(t:mission("stash").State.offered)
   t:expectDialog("That package was worth the wait.", "Bruno")
   t:expectQuestion(t:mission("stash").Question.acceptJob)
end)

test("package: the trip is saved: continuing after the docking still has the package", function(t)
   local m = docked(t)
   t:exitScene(1, 3)
   t:restartGame()
   t:expectMission("package")
   t:expectState(m.State.carrying)
end)

test("package: after it, the world's space is back; the station gives nothing (halfway does)", function(t)
   home(t)
   t:land("villagePad")
   t.engine:callLua("lua_startMission", "freePlay")
   t:takeOff()
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expect(t.engine:getObject("hammer1") ~= nil, "the enemies are back")
   t:expect(t.engine:getObject("shieldPower1") == nil, "no shield power unit")
   t:event(station, "DsDk")
   t:expectItem("badMineSolution", "unknown")
end)
