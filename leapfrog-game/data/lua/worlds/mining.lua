-- The mining world (the home planet): its scenes' pads, services and
-- repeatable jobs, whatever mission is running, also in free play.
-- mission_script.lua calls this for the scenes it maps to "mining", after
-- lua/common.lua and before the mission. See common.lua for the rules.

local mining = {}


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
-- Scene setup, before the mission's

mining.setupScene = function()

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
end


-- ------------------------------------------------------------------------
-- Events, before the mission's handler

mining.eventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then
      if actorName == "siloLandingPad" and eventId == "LpLL" then
         offerLegRepair()
      end
   end

   -- Answers to c_askQuestion come as QsYs / QsNo (QsWd: took off
   -- instead), with the question id as parameter
   if parameter1 == Question.repairLegs then
      if eventId == "QsYs" then
         answerLegRepair(true)
      elseif eventId == "QsNo" then
         answerLegRepair(false)
      end
   end
end

return mining
