-- Mission 2, the dark beacons: the worked example of
-- documentation/lua_guide.md

-- Starts the mission from the main menu's test row, and returns its states
-- and questions
local function start(t)
   t:newGame()
   local labels = t:testStarts()
   for i, label in ipairs(labels) do
      if label == "Beacons, job offered" then
         t:menuTestStart(i)
      end
   end
   t:expectMission(2)
   local m = t:mission(2)
   return m.State, m.Question
end

local function acceptJob(t, S, Q)
   t:land("siloLandingPad")
   t:answer(Q.takeJob, true)
   t:expectState(S.firstBeacon)
   t:takeOff()
end

local function lightAll(t)
   for _, pad in ipairs({ "landingPad2", "landingPad3", "workshopPad" }) do
      t:land(pad)
      t:takeOff()
   end
end

test("mission 2: the job is offered and the beacons are dark", function(t)
   local S, Q = start(t)
   t:expectState(S.offered)
   t:expectDialog("Three of the old beacons", "Bruno")
   t:expectQuestionQueued(Q.takeJob)
   t:expectProperty("landingPad2", 2, 0)
   t:expectProperty("landingPad3", 2, 0)
   t:expectProperty("workshopPad", 2, 0)
   t:expectMap("landingPad2", "siloHome", "hollow")
end)

test("mission 2: declining, then landing at the silo asks again", function(t)
   local S, Q = start(t)
   t:land("siloLandingPad")
   t:answer(Q.takeJob, false)
   t:expectState(S.offered)
   t:takeOff()
   t:land("siloLandingPad")
   t:expectQuestion(Q.takeJob)
   t:answer(Q.takeJob, true)
   t:expectState(S.firstBeacon)
end)

test("mission 2: the beacons are lit in order, each with a landing", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   t:expectMap("landingPad2", "siloHome", "flashingSlow")

   t:land("landingPad2")
   t:expectState(S.secondBeacon)
   t:expectHud("Beacon lit")
   t:expectProperty("landingPad2", 2, 1)
   t:expectProperty("landingPad2", 4, 2)
   t:expectMap("landingPad2", "siloHome", "filled")
   t:expectMap("landingPad3", "siloHome", "flashingSlow")
   t:takeOff()

   t:land("landingPad3")
   t:expectState(S.thirdBeacon)
end)

test("mission 2: landing on a beacon out of turn does nothing", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   t:land("landingPad3")
   t:expectState(S.firstBeacon)
   t:expectProperty("landingPad3", 2, 0)
   t:takeOff()
end)

test("mission 2: landing again on a lit beacon doesn't count twice", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   t:land("landingPad2")
   t:takeOff()
   t:land("landingPad2")
   t:expectState(S.secondBeacon)
end)

test("mission 2: Bruno calls when all are lit, once", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   lightAll(t)
   t:expectState(S.collect)
   t:clearDialog()
   t:wait(1)
   t:expectNoDialog("Come to the Orb")
   t:wait(2)
   t:expectEqual(t:countDialog("Come to the Orb"), 1, "Bruno's calls")
   t:expectDialog("Come to the Orb", "Bruno")
   t:expectMap("oreLanding", "noName", "flashingSlow")
end)

test("mission 2: the Orb pays, once", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   lightAll(t)
   local credits = t:property("leapfrog1", 8)

   t:land("oreLanding")
   t:expectState(S.done)
   t:expectProperty("leapfrog1", 8, credits + 150)
   t:expectHud("Mission complete")
   t:takeOff()

   t:land("oreLanding")
   t:expectProperty("leapfrog1", 8, credits + 150)
end)

test("mission 2: progress survives a launch, quitting and continuing", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   t:land("landingPad2")
   t:expectState(S.secondBeacon)
   t:takeOff()

   -- The scene exit saves the state
   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectSaved("mission", 2)
   t:expectSaved("state", S.secondBeacon)

   t:restartGame()
   t:menuContinue()
   t:expectMission(2)
   t:expectState(S.secondBeacon)
end)

test("mission 2: back at home the scene is rebuilt from the state", function(t)
   local S, Q = start(t)
   acceptJob(t, S, Q)
   t:land("landingPad2")
   t:takeOff()
   t:launch("launchSite1")

   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:reentry("onSite", 0)
   t:expectScene("home_scene.xml")

   t:expectState(S.secondBeacon)
   t:expectProperty("landingPad2", 2, 1)
   t:expectProperty("landingPad3", 2, 0)
   t:expectMap("landingPad2", "siloHome", "filled")
   t:expectMap("landingPad3", "siloHome", "flashingSlow")
end)

test("mission 2: a mission's holds answers for it, whatever its number", function(t)
   start(t)
   t:expect(t.engine:callLuaResults("missionHolds", "swarm")[1] == false, "mission 2 holds nothing")
   t:mission(2).holds = function(what)
      return what == "swarm"
   end
   t:expect(t.engine:callLuaResults("missionHolds", "swarm")[1] == true, "mission 2 holds the swarm")
   t:mission(2).holds = nil
end)
