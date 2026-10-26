-- The world: everything that is there whatever mission is running, also
-- when none is (currentMission == 0, free play). mission_script.lua calls
-- world.setupScene and world.eventHandler for every scene, before the
-- mission's own functions.
--
-- Rules for this file and the mission scripts:
-- - Only mission_script.lua calls c_clearAllTriggersAndEvents (before the
--   world and the mission set up), so neither wipes the other's handlers.
-- - Register events with listen(actorName, eventId), not
--   c_registerEventHandler: when the world and a mission both listen to
--   the same event (e.g. the silo's LpLL), it is registered once, and each
--   handler is called once.
-- - Question ids (c_askQuestion): the world uses 100 and up, missions
--   1 to 99.

local world = {}


-- ------------------------------------------------------------------------
-- Message senders: who's who, for the colour of the message bubble frames.
-- Senders not set here stay phosphor green. A message can also be given
-- its own affiliation as a sixth argument to c_addDialogMessage. Set at
-- every scene setup, so it follows the story after a load (in Act 3: a
-- mission sets c_setSenderAffiliation("<pirate>", "friendly")).

local function setSenders()
   c_setSenderAffiliation("Edith", "leapfrog")
   c_setSenderAffiliation("Silo", "leapfrog")
   c_setSenderAffiliation("DEBUG", "leapfrog")
   c_setSenderAffiliation("Leapfrog", "leapfrog")
   c_setSenderAffiliation("Bruno", "neutral")

   -- Edith is on the Leapfrog's side, but her messages get a colour of
   -- their own so they don't look like the pilot's white ones
   c_setSenderColor("Edith", "ff80d5")
end


-- ------------------------------------------------------------------------
-- Leg repair at home (the silo's pad). Leapfrog property 10 is the lowest
-- leg joint health, 0..1 (0 = a joint is limp); writing it raises every
-- joint to at least that value. A full repair costs credits (property 8).
-- Without enough credits the silo patches limp joints for free, so the
-- legs hold but stay weak.

local questionRepairLegs = 100   -- c_askQuestion id
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
      c_askQuestion(questionRepairLegs)
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
-- Scene setup, every scene, before the mission's

world.setupScene = function()

   setSenders()

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
-- Events, every scene, before the mission's handler

world.eventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then
      if actorName == "siloLandingPad" and eventId == "LpLL" then
         offerLegRepair()
      end
   end

   -- Answers to c_askQuestion come as QsYs / QsNo (QsWd: took off
   -- instead), with the question id as parameter
   if parameter1 == questionRepairLegs then
      if eventId == "QsYs" then
         answerLegRepair(true)
      elseif eventId == "QsNo" then
         answerLegRepair(false)
      end
   end
end

return world
