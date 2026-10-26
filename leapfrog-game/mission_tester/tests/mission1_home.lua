-- Mission 1 in the home scene: Bruno's offer, the route to the Orb, the Orb

local function m1(t)
   local m = t:mission(1)
   return m.State, m.Question
end

test("mission 1: a new game plays Bruno's call and asks for the job", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:expectState(S.offered)
   t:expectDialog("I hear you are in bad shape", "Bruno")
   t:expectDialog("Do you want the job?", "Bruno")
   t:expectQuestion(Q.acceptJob)

   -- Nothing of the route yet: no fuel in Bruno's pads, launch site hidden
   t:expectProperty("landingPad2", 4, 0)
   t:expectProperty("landingPad3", 4, 0)
   t:expectMap("landingPad2", "siloHome", "hollow")
   t:expectMapShown("launchSite1", "mainTank", false)
end)

test("mission 1: accepting puts fuel in the pads and shows the route", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:clearDialog()
   t:answer(Q.acceptJob, true)
   t:expectState(S.accepted)
   t:expectDialog("I take it", "Leapfrog")
   t:expectDialog("Just follow the markers", "Bruno")
   t:expectNoQuestion()

   -- Free fuel (service 2) in both pads
   t:expectProperty("landingPad2", 4, 2)
   t:expectProperty("landingPad3", 4, 2)
   t:expectProperty("landingPad2", 3, 15)

   -- First stop flashes
   t:expectMap("landingPad2", "siloHome", "flashingSlow")
   t:expectMap("landingPad3", "siloHome", "hollow")
   t:expectMap("oreLanding", "noName", "hollow")
end)

test("mission 1: declining, then landing at the silo asks again", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:clearDialog()
   t:answer(Q.acceptJob, false)
   t:expectDialog("I'm going to think about it", "Leapfrog")
   t:expectDialog("Land back here", "Bruno")
   t:expectState(S.offered)
   t:expectNoQuestion()

   t:land("siloLandingPad")
   t:expectDialog("Changed your mind?", "Bruno")
   t:expectQuestion(Q.acceptJob)

   t:answer(Q.acceptJob, true)
   t:expectState(S.accepted)
end)

test("mission 1: landing at the silo while the offer is open doesn't ask twice", function(t)
   t:newGame()
   local _, Q = m1(t)
   t:clearDialog()
   t:land("siloLandingPad")
   t:expectNoDialog("Do you want the job?")
   t:expectQuestion(Q.acceptJob)
end)

test("mission 1: taking off withdraws the offer; the next landing asks again", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:land("siloLandingPad")
   t:takeOff()
   t:expectNoQuestion()
   t:expectState(S.offered)

   t:clearDialog()
   t:land("siloLandingPad")
   t:expectDialog("Do you want the job?", "Bruno")
   t:expectQuestion(Q.acceptJob)
end)

test("mission 1: flying to the Orb without the job, Bruno asks there", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:answer(Q.acceptJob, false)
   t:clearDialog()

   t:land("oreLanding")
   t:expectDialog("You came anyway", "Bruno")
   t:expectQuestion(Q.acceptJob)

   -- Accepting on the Orb's pad goes straight to the arrival
   t:answer(Q.acceptJob, true)
   t:expectState(S.atOrb)
   t:expectDialog("I agree, you need that job.", "Edith")
end)

test("mission 1: the route moves on pad by pad", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:answer(Q.acceptJob, true)

   t:land("landingPad2")
   t:expectMap("landingPad2", "siloHome", "hollow")
   t:expectMap("landingPad3", "siloHome", "flashingSlow")
   t:takeOff()

   t:land("landingPad3")
   t:expectMap("landingPad3", "siloHome", "hollow")
   t:expectMap("oreLanding", "noName", "flashingSlow")
   t:takeOff()

   t:land("oreLanding")
   t:expectState(S.atOrb)
   t:expectMap("oreLanding", "noName", "hollow")
end)

test("mission 1: pads taken out of order still lead to the Orb", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:answer(Q.acceptJob, true)

   t:land("landingPad3")
   t:expectMap("oreLanding", "noName", "flashingSlow")
   t:takeOff()

   -- Going back to pad 2 doesn't move the route backwards
   t:land("landingPad2")
   t:expectMap("oreLanding", "noName", "flashingSlow")
   t:expectMap("landingPad2", "siloHome", "hollow")
end)

test("mission 1: Bruno's pads give fuel, once", function(t)
   t:newGame()
   local _, Q = m1(t)
   t:answer(Q.acceptJob, true)

   local before = t:property("leapfrog1", 7)
   t:land("landingPad2")
   t:expectProperty("leapfrog1", 7, before + 15)
   t:expectProperty("landingPad2", 3, 0)        -- the pad's supply is used up
   t:takeOff()

   t:land("landingPad2")
   t:expectProperty("leapfrog1", 7, before + 15)
end)

test("mission 1: arriving at the Orb: Edith, the taskgiver's fuel, the launch site", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:answer(Q.acceptJob, true)
   t:setProperty("leapfrog1", 7, 3)            -- nearly out of fuel

   t:clearDialog()
   t:land("oreLanding")
   t:expectState(S.atOrb)
   t:expectHud("Fuel from the taskgiver")
   t:expectProperty("leapfrog1", 7, 25)        -- topped up to the start fuel
   t:expectDialog("I agree, you need that job.", "Edith")
   t:expectDialog("Not enough fuel. You'll be stranded.", "Edith")

   -- The launch site appears on the map once Edith has had her say
   t:expectMapShown("launchSite1", "mainTank", false)
   t:wait(12)
   t:expectMapShown("launchSite1", "mainTank", true)
end)

test("mission 1: landing at the Orb again doesn't replay the arrival", function(t)
   t:newGame()
   local _, Q = m1(t)
   t:answer(Q.acceptJob, true)
   t:land("oreLanding")
   t:takeOff()
   t:clearDialog()
   t:land("oreLanding")
   t:expectNoDialog("I agree, you need that job.")
end)

test("mission 1: launching from home goes to deep space and saves", function(t)
   t:newGame()
   local S, Q = m1(t)
   t:answer(Q.acceptJob, true)
   t:land("oreLanding")
   t:takeOff()

   t:launch("launchSite1")
   t:expectScene("deep_space_scene.xml")
   t:expectSaved("scene", "deep_space_scene.xml")
   t:expectSaved("exitScene", "home_scene.xml")
   t:expectSaved("state", S.atOrb)
end)
