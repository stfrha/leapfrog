-- Script structure: loading, new game set-up, the rules in lua/common.lua

test("scripts: a new game loads and starts at home in mission 1", function(t)
   t:newGame()
   t:expectScene("home_scene.xml")
   t:expectMission(1)
   t:expectState(t:mission(1).firstState)
   t:expect(t:global("lfDefined") == true, "the Leapfrog is created")
   t:expect(t:global("newGameStarting") == false, "newGameStarting is cleared after the first set-up")
end)

test("scripts: the Leapfrog starts with the new game's resources", function(t)
   t:newGame()
   t:expectProperty("leapfrog1", 7, 25)     -- fuel
   t:expectProperty("leapfrog1", 8, 20)     -- credits
   t:expectProperty("leapfrog1", 10, 1)     -- legs sound
end)

test("scripts: each event is registered once per set-up", function(t)
   t:newGame()
   -- Both the mining world and mission 1 listen to the silo's pad
   t:expectListening("siloLandingPad", "LpLL", 1)
   t:expectListening("oreLanding", "LpLL", 1)
   t:expectListening("launchSite1", "LsLL", 1)
end)

test("scripts: the first scene is saved with the mission's first state", function(t)
   t:newGame()
   t:expectSaved("scene", "home_scene.xml")
   t:expectSaved("mission", 1)
   t:expectSaved("state", t:mission(1).firstState)
   t:expectSaved("exitType", -1)
end)

test("scripts: the menu's new game in a running game starts over", function(t)
   t:newGame()
   local S = t:mission(1).State
   t:land("siloLandingPad")
   t:answer(t:mission(1).Question.acceptJob, true)
   t:expectState(S.accepted)
   t:menuNewGame()
   t:expectScene("home_scene.xml")
   t:expectState(S.offered)
   t:expectDialog("Do you want the job?", "Bruno")
   t:expectQuestionQueued(t:mission(1).Question.acceptJob)
end)
