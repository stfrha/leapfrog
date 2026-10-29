-- The mining world (the home planet): its scenes' pads, services and
-- repeatable jobs, whatever mission is running, also in free play.
-- mission_script.lua calls this for the scenes it maps to "mining", after
-- lua/common.lua and before the mission. See common.lua for the rules.

local mining = {}

local swarm = require('swarm')
local debt = require('debt')

-- ------------------------------------------------------------------------
-- Leg repair at home (the silo's pad). Leapfrog property 10 is the lowest
-- leg joint health, 0..1 (0 = a joint is limp); writing it raises every
-- joint to at least that value. A full repair costs credits (property 8).
-- Without enough credits the silo patches limp joints for free, so the
-- legs hold but stay weak.

-- Question ids (c_askQuestion): the mining world uses 100 to 199
local Question = {
   repairLegs = 100,
}
mining.Question = Question
local legRepairCost = 0
local legFieldFixHealth = 0.3

local function legRepairCostFor(legHealth)
   -- 20 credits for a strain, up to 60 for a limp leg
   return math.ceil(20 + 40 * (1 - legHealth))
end

local function offerLegRepair()
   local legs = c_getObjectProperty("leapfrog1", 10)
   if legs >= 1 then
      return
   end

   legRepairCost = legRepairCostFor(legs)
   -- Properties are floats; whole credits read better
   local credits = math.floor(c_getObjectProperty("leapfrog1", 8))

   if credits >= legRepairCost then
      c_addDialogMessage("Leg actuators damaged. Full repair for " .. legRepairCost .. " credits?", "Silo", true, 30, 0)
      c_askQuestion(Question.repairLegs)
   elseif legs < legFieldFixHealth then
      c_addDialogMessage("Leg actuators damaged. A full repair is " .. legRepairCost .. " credits, you have " .. credits .. ".", "Silo", true, 30, 0)
      c_addDialogMessage("I've patched the joints so they hold. Take it easy on the landings.", "Silo", true, 90, 0)
      c_setObjectProperty("leapfrog1", 10, legFieldFixHealth)
      c_hudLog("Legs patched")
   end
   -- Otherwise (can't pay, and the legs are already at least patched) the
   -- silo has nothing to offer: say nothing
end

local function answerLegRepair(yes)
   if not yes then
      c_addDialogMessage("No repair.", "Leapfrog", false, 30, 0)
      return
   end

   -- Read again: credits may have changed while the question was open
   local credits = c_getObjectProperty("leapfrog1", 8)
   if credits >= legRepairCost then
      c_setObjectProperty("leapfrog1", 8, credits - legRepairCost)
      c_setObjectProperty("leapfrog1", 10, 1)
      c_addDialogMessage("Yes, fix them.", "Leapfrog", false, 30, 0)
      c_hudLog("Legs repaired: " .. legRepairCost .. " credits")
   else
      c_addDialogMessage("Not enough credits any more.", "Silo", true, 30, 0)
   end
end


-- ------------------------------------------------------------------------
-- Re-entry outcomes (lua_reentryOutcome, when the orbit scene exits).
-- Down off the site: a recovery crew picks the Leapfrog up and sets it
-- down on the site's pad (scene_navigator.lua), for a fee by the distance (a site's marker is 100
-- planet units wide); what the credits don't cover, Bruno lends.
-- Bounced off the atmosphere: back in deep space, where the strippers
-- hold off for a while. Burned up: the game goes back to the latest save.
-- What is said comes at the next scene's setup.

local recoveryBaseFee = 20
local recoveryFeePerUnit = 0.4      -- 40 credits per marker width off
local bounceGraceTicks = 600        -- 10 s before the swarm starts

local arrivalLines = {}             -- { sender, text } for the next setup
local arrivalHud = {}               -- HUD log lines for the next setup
local bouncedBack = false           -- arriving in deep space by a bounce

local function recoveryFee(distance)
   return math.ceil(recoveryBaseFee + recoveryFeePerUnit * distance)
end
mining.recoveryFee = recoveryFee

mining.reentryOutcome = function(r)
   if r.outcome == "offSite" then
      local fee = recoveryFee(r.distance)
      arrivalLines[#arrivalLines + 1] = { "Recovery",
         "We picked you up out in the scrub and set you down on the pad. That's " .. fee .. " credits." }
      arrivalHud[#arrivalHud + 1] = "Recovery: " .. fee .. " credits"
      debt.charge(fee, "the recovery crew", true)

   elseif r.outcome == "bounced" then
      arrivalHud[#arrivalHud + 1] = "Bounced off the atmosphere"
      bouncedBack = true

   elseif r.outcome == "burnedUp" then
      arrivalHud[#arrivalHud + 1] = "Burned up on re-entry. Back to the latest save."
   end
end

local function sayArrival()
   for _, line in ipairs(arrivalLines) do
      c_addDialogMessage(line[2], line[1], true, 30, 0)
   end
   for _, text in ipairs(arrivalHud) do
      c_hudLog(text, true)
   end
   arrivalLines = {}
   arrivalHud = {}
end

local swarmTimer = -1

local function startSwarm()
   swarm.start("swarm", "stripper_swarm.xml", function(name)
      c_hudLog("The strippers have taken everything", true)
   end)
end


-- ------------------------------------------------------------------------
-- Scene setup, before the mission's

mining.setupScene = function()

   sayArrival()

   if currentScene == "home_scene.xml" then

      -- Starting at home (a new game, or a saved game without a position):
      -- hover just above the silo's landing pad (the pad's top is at
      -- y = 588; in landing mode the leapfrog's origin is about 14 above
      -- its feet)
      if lfDefined == false and useInitialLfPos == true then
         c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 2736, 503)

         -- Reconfigure leapfrog to landing mode
         c_setObjectProperty("leapfrog1", 0, 2)
         lfDefined = true
      end

      -- The silo's pad is home: white on the map (landing pads are
      -- neutral by default). "siloHome" is the pad's box in home_scene.xml.
      c_setObjectMapAffiliation("siloLandingPad", "siloHome", "leapfrog")

      -- Landed at home: the silo offers to repair the legs
      listen("siloLandingPad", "LpLL")
   end

   if currentScene == "deep_space_scene.xml" then
      if bouncedBack or swarmTimer >= 0 then
         -- Back from a bounce: a breather before the swarm comes (again
         -- at a setup before it has started)
         bouncedBack = false
         swarmTimer = c_startSceneTimer(bounceGraceTicks)
      else
         startSwarm()
      end
   else
      swarmTimer = -1
   end

end


-- ------------------------------------------------------------------------
-- Events, before the mission's handler

mining.eventHandler = function(eventId, actorName, parameter1)

   if eventId == "ScTO" and swarmTimer >= 0 and parameter1 == swarmTimer then
      swarmTimer = -1
      startSwarm()
   end

   if currentScene == "home_scene.xml" then
      if actorName == "siloLandingPad" and eventId == "LpLL" then
         offerLegRepair()
      end
   end

   -- Answers to c_askQuestion come as QsYs / QsNo, with the question id
   -- as parameter (asked on the silo's pad, so it opens at once, or after
   -- a question asked before it)
   if parameter1 == Question.repairLegs then
      if eventId == "QsYs" then
         answerLegRepair(true)
      elseif eventId == "QsNo" then
         answerLegRepair(false)
      end
   end
end

return mining
