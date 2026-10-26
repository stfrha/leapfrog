-- The old test mission in landing_scene.xml (the main menu's "test
-- landing"), states testLandOnPad ... testEnded

test("test mission: from the landing pad to the reward", function(t)
   t:menuTestLanding()
   local S = t:mission(1).State
   t:expectScene("landing_scene.xml")
   t:expectState(S.testLandOnPad)
   t:expectDialog("Try to land on the landing pad", "DEBUG")

   t:land("landing_pad1")
   t:expectState(S.testTakeOff)
   t:takeOff()
   t:expectState(S.testFreeSpring)

   t:event("coDestroyable", "BrBR")
   t:expectState(S.testDestroyBomb)

   local credits = t:property("leapfrog1", 8)
   t:event("bomb1", "ExEX")
   t:expectState(S.testFindTreasure)
   t:expectProperty("leapfrog1", 8, credits + 200)
end)

test("test mission: the bomb too early while freeing the spring ends it", function(t)
   t:menuTestLanding()
   local S = t:mission(1).State
   t:land("landing_pad1")
   t:takeOff()
   t:expectState(S.testFreeSpring)
   t:event("bomb1", "ExEX")
   t:expectDialog("Mission FAIL", "DEBUG")
   t:expectState(S.testEnded)
end)

test("test mission: the bomb too early on the pad ends it", function(t)
   t:menuTestLanding()
   local S = t:mission(1).State
   t:event("bomb1", "ExEX")
   t:expectState(S.testEnded)
end)
