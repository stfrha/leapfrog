-- The mining world: the silo's leg repair (lua/worlds/mining.lua)

local function setup(t, legs, credits)
   t:newGame()
   -- Out of Bruno's opening, so his offer doesn't get in the way
   t:answer(t:mission(1).Question.acceptJob, true)
   t:damageLegs(legs)
   t:setProperty("leapfrog1", 8, credits)
   t:clearDialog()
   return t:module("mining").Question
end

test("repair: damaged legs and enough credits: the silo asks, yes repairs", function(t)
   local Q = setup(t, 0, 70)
   t:land("siloLandingPad")
   t:expectDialog("Full repair for 60 credits?", "Silo")
   t:expectQuestion(Q.repairLegs)
   t:answer(Q.repairLegs, true)
   t:expectProperty("leapfrog1", 10, 1)
   t:expectProperty("leapfrog1", 8, 10)
   t:expectHud("Legs repaired: 60 credits")
end)

test("repair: the price follows the damage", function(t)
   local Q = setup(t, 0.5, 70)
   t:land("siloLandingPad")
   t:expectDialog("Full repair for 40 credits?", "Silo")
   t:answer(Q.repairLegs, true)
   t:expectProperty("leapfrog1", 8, 30)
end)

test("repair: no repair leaves the legs and the credits", function(t)
   local Q = setup(t, 0.5, 70)
   t:land("siloLandingPad")
   t:answer(Q.repairLegs, false)
   t:expectDialog("No repair.", "Leapfrog")
   t:expectProperty("leapfrog1", 10, 0.5)
   t:expectProperty("leapfrog1", 8, 70)
end)

test("repair: a limp leg and too few credits: a free patch, no question", function(t)
   setup(t, 0, 10)
   t:land("siloLandingPad")
   t:expectDialog("A full repair is 60 credits, you have 10.", "Silo")
   t:expectDialog("I've patched the joints", "Silo")
   t:expectNoQuestion()
   t:expectProperty("leapfrog1", 10, 0.3)
   t:expectHud("Legs patched")
end)

test("repair: patched legs and too few credits: the silo says nothing", function(t)
   setup(t, 0.3, 10)
   t:land("siloLandingPad")
   t:expectNoDialog()
   t:expectNoQuestion()
end)

test("repair: sound legs: the silo says nothing", function(t)
   setup(t, 1, 70)
   t:land("siloLandingPad")
   t:expectNoDialog()
   t:expectNoQuestion()
end)

test("repair: works in free play (no mission)", function(t)
   setup(t, 0, 70)
   t:setGlobal("currentMission", 0)
   t:setupScene()
   t:expectListening("siloLandingPad", "LpLL", 1)
   t:land("siloLandingPad")
   t:expectQuestion(t:module("mining").Question.repairLegs)
end)

known("repair: the repair offer doesn't lose Bruno's open job offer",
   "c_askQuestion replaces an open question without telling its script, so "
   .. "mission 1 still thinks its offer is open and never asks again "
   .. "(fix: a question queue, or a withdraw event for the replaced question)",
   function(t)
      t:newGame()
      local Q1 = t:mission(1).Question
      local QM = t:module("mining").Question
      -- A crash during Bruno's call, then landing at the silo with
      -- credits for the repair: its question replaces Bruno's
      t:damageLegs(0)
      t:setProperty("leapfrog1", 8, 70)
      t:land("siloLandingPad")
      t:expectQuestion(QM.repairLegs)
      t:answer(QM.repairLegs, true)
      t:takeOff()
      t:clearDialog()
      t:land("siloLandingPad")
      t:expectQuestion(Q1.acceptJob)
   end)
