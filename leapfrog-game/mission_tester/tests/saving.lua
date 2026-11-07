-- Saving and continuing (quit the game, start it again)

test("saving: continuing after launching resumes in deep space", function(t)
   t:stashJobTaken()
   local m = t:mission("stash")
   t:launch("launchSite1")

   t:restartGame()
   t:expectScene("debris_belt_scene.xml")
   t:expectMission("stash")
   t:expectState(m.State.hunting)
end)

test("saving: resources and leg health are saved at a scene exit", function(t)
   t:stashJobTaken()
   local m = t:mission("stash")
   t:setProperty("leapfrog1", 8, 123)
   t:damageLegs(0.4)
   t:launch("launchSite1")

   t:restartGame()
   t:expectProperty("leapfrog1", 8, 123)
   t:expectProperty("leapfrog1", 10, 0.4)
end)

known("saving: continuing at home keeps the mission's progress",
   "the game saves only at scene exits and a mission's first scene; quitting "
   .. "at home before leaving it and continuing starts the mission over "
   .. "(Bruno's call again, state offered)",
   function(t)
      t:newGame()
      local m = t:mission("beacons")
      t:land("siloLandingPad")
   t:answer(m.Question.takeJob, true)
      t:restartGame()
      t:expectState(m.State.firstBeacon)
   end)

test("saving: a save from before missions had names continues in the right mission", function(t)
   -- Old saves hold the mission's number: 1 beacons, 2 stash, 0 free play
   for number, name in pairs({ [0] = "freePlay", [1] = "beacons", [2] = "stash" }) do
      t:newGame()
      t.engine.file.mission = tostring(number)
      t.engine.file.scene = "deep_space_scene.xml"
      t.engine.file.sceneType = 1
      t.engine.file.exitScene, t.engine.file.exitType, t.engine.file.exitParam = "home_scene.xml", 0, 1
      t:restartGame()
      t:expectMission(name)
   end
end)
