-- Common: what is true in every scene of every world, whatever mission is
-- running, also when none is (currentMission == 0, free play).
--
-- The scripts, in the order mission_script.lua calls them for each scene
-- setup and each event:
--   lua/common.lua          this file
--   lua/worlds/<world>.lua  the scene's world (scene -> world table in
--                           mission_script.lua): its pads, services and
--                           repeatable jobs, e.g. the silo's leg repair
--   lua/mission_<n>.lua     the running mission's story
--
-- Rules for all of them:
-- - Only mission_script.lua calls c_clearAllTriggersAndEvents (before
--   anything sets up), so no script wipes another's handlers.
-- - Register events with listen(actorName, eventId), not
--   c_registerEventHandler: when two scripts listen to the same event
--   (e.g. the silo's LpLL), it is registered once and each handler is
--   called once.
-- - Question ids (c_askQuestion): missions 1 to 99, worlds 100 to 999
--   (mining 100 to 199, the next world 200 to 299, ...), common 1000 and up.
-- - States (currentState) are declared by name in the mission's State
--   table and referred to by name.

local common = {}


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
   c_setSenderAffiliation("Recovery", "neutral")

   -- Edith is on the Leapfrog's side, but her messages get a colour of
   -- their own so they don't look like the pilot's white ones
   c_setSenderColor("Edith", "ff80d5")
end


-- ------------------------------------------------------------------------
-- Scene setup, every scene, first

common.setupScene = function()
   setSenders()
end


-- ------------------------------------------------------------------------
-- Events, every scene, first

common.eventHandler = function(eventId, actorName, parameter1)
end

return common
