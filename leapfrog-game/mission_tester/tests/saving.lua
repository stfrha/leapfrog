-- Saving and continuing (quit the game, start it again)

test("saving: continuing after launching resumes in deep space", function(t)
   t:newGame()
   local m = t:mission(1)
   t:land("siloLandingPad")
   t:answer(m.Question.acceptJob, true)
   t:land("oreLanding")
   t:takeOff()
   t:launch("launchSite1")

   t:restartGame()
   t:expectScene("deep_space_scene.xml")
   t:expectMission(1)
   t:expectState(m.State.atOrb)
end)

test("saving: resources and leg health are saved at a scene exit", function(t)
   t:newGame()
   local m = t:mission(1)
   t:land("siloLandingPad")
   t:answer(m.Question.acceptJob, true)
   t:land("oreLanding")
   t:takeOff()
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
      local m = t:mission(1)
      t:land("siloLandingPad")
   t:answer(m.Question.acceptJob, true)
      t:restartGame()
      t:expectState(m.State.accepted)
   end)
