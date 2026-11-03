-- Mission 2: the dark beacons. The worked example in
-- documentation/lua_guide.md, so keep the guide in step with it (the mission
-- tester checks that the guide's code is still here).
--
-- Bruno pays the pilot to relight two dead beacons in the home scene, in
-- order, and to collect the money at the Ore. Mission 1 hands over to it when the
-- pilot is back at the silo; the main menu's test row also starts it
-- ("Beacons, job offered").

local mission2 = {}

-- currentState is all that is saved of a mission, so every step the player
-- must not lose has a state of its own. The beacon states run in order: the
-- setup compares them with < and >.
local State = {
   offered      = 10,  -- Bruno has offered the job
   firstBeacon  = 11,  -- relight the first beacon
   secondBeacon = 12,
   collect      = 13,  -- all lit, collect the money at the Ore
   done         = 14,
}
mission2.State = State

-- A new game or a test start begins here
mission2.firstState = State.offered

-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   takeJob = 2,
}
mission2.Question = Question

-- Landing pad properties (see src/landingpad.h)
local Pad = {
   lfLanded    = 1,
   active      = 2,
   fuelSupply  = 3,
   fuelService = 4,    -- 2 = free fuel
}

-- Leapfrog property 8 = credits
local creditsProperty = 8

-- The beacons are pads of home_scene.xml. Each is lit in the state after its
-- own. "body" is the pad's shape, which is its square on the map, and "lamp"
-- the sprite with its blinking lamp.
local beacons = {
   { pad = "landingPad2", body = "siloHome", lamp = "landingPad2Lamp", state = State.firstBeacon },
   { pad = "landingPad3", body = "siloHome", lamp = "landingPad3Lamp", state = State.secondBeacon },
}
mission2.beacons = beacons

local blinkPeriod = 1.2        -- seconds, as animatePeriod in the scene
local lampOffFrame = 1         -- gen_landing_pad_blink: 0 on, 1 off

local beaconFuel = 15          -- free fuel in a lit beacon's pad
local reward = 150             -- credits
local reminderTicks = 120      -- frames (60 a second)
local repairTicks = 180        -- a beacon takes 3 s to repair

local reminderTimer = -1       -- id of the running timer, -1 if none
local repairTimer = -1
local repairingPad = nil       -- the pad being repaired


-- ------------------------------------------------------------------------
-- Scene setup

-- Makes the beacons what currentState says. An inactive pad sends no LpLL,
-- so the next beacon is watched through its lfLanded property, which changes
-- active or not. "OpTr" arrives with the property id as parameter.
local function setupBeacons()
   for _, beacon in ipairs(beacons) do
      local lit = currentState > beacon.state
      local isNext = currentState == beacon.state

      if lit then
         c_setObjectProperty(beacon.pad, Pad.active, 1)
         c_animateSprite(beacon.lamp, blinkPeriod)
         c_setObjectMapState(beacon.pad, beacon.body, 1)     -- filled
      else
         c_setObjectProperty(beacon.pad, Pad.active, 0)
         c_animateSprite(beacon.lamp, 0, lampOffFrame)
         c_setObjectMapState(beacon.pad, beacon.body, isNext and 2 or 0)  -- flashing or hollow
      end

      if isNext then
         c_registerPropertyTrigger(beacon.pad, 0, Pad.lfLanded, "insideRange", 1, 1)
         listen(beacon.pad, "OpTr")
      end
   end
end

-- Called for every new scene, and again by each change of state below.
-- Triggers and event listeners have been cleared, so everything that is
-- needed in this state is set up here, from currentState alone.
mission2.setupMissionStateScene = function()

   if currentScene ~= "home_scene.xml" then
      return
   end

   repairingPad = nil
   repairTimer = -1
   setupBeacons()

   if currentState == State.offered then
      listen("siloLandingPad", "LpLL")
      c_addDialogMessage("Two of the old beacons have gone dark. I pay to have them lit.", "Bruno", true, 60, 0)
      c_askQuestion(Question.takeJob, "Will you do it?", "Bruno", "Touch down and let's talk.")

   elseif currentState == State.collect then
      listen("oreLanding", "LpLL")
      c_setObjectMapState("oreLanding", "noName", 2)
      reminderTimer = c_startSceneTimer(reminderTicks)
   end
end


-- ------------------------------------------------------------------------
-- Events

-- Goes to a new state and sets its scene up again
local function enterState(state)
   currentState = state
   lua_setupMissionStateScene()
end

local function answerJob(yes)
   if yes then
      c_addDialogMessage("I'll do it.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("The next one flashes on your map. Land on it and stay put while it's repaired.", "Bruno", true, 120, 0)
      enterState(State.firstBeacon)
   else
      c_addDialogMessage("Not now.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Land at the silo if you change your mind.", "Bruno", true, 120, 0)
   end
end

-- Landed on the beacon being watched. lfLanded is set again if a foot
-- bounces, so a repair under way is left alone.
local function startRepair(padName)
   if repairingPad ~= nil then
      return
   end
   repairingPad = padName
   c_hudLog("Repairing beacon")
   repairTimer = c_startSceneTimer(repairTicks)
end

-- The repair is done if the Leapfrog stayed. The next setup lights the lamp
-- and shows the next beacon. Activating the pad gives free fuel at once,
-- since the Leapfrog stands on it.
local function finishRepair()
   local padName = repairingPad
   repairingPad = nil
   repairTimer = -1

   if c_getObjectProperty(padName, Pad.lfLanded) ~= 1 then
      c_addDialogMessage("You left before it was done. Land there again and stay put.", "Bruno", true, 0, 0)
      return
   end

   c_setObjectProperty(padName, Pad.fuelSupply, beaconFuel)
   c_setObjectProperty(padName, Pad.fuelService, 2)
   c_hudLog("Beacon lit")
   enterState(currentState + 1)
end

local function collectReward()
   local credits = c_getObjectProperty("leapfrog1", creditsProperty)
   c_setObjectProperty("leapfrog1", creditsProperty, credits + reward)
   c_setObjectMapState("oreLanding", "noName", 0)
   c_hudLog("Mission complete: " .. reward .. " credits")
   c_addDialogMessage("Here's your money. Don't spend it all on fuel.", "Bruno", true, 30, 0)
   enterState(State.done)
end

mission2.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene ~= "home_scene.xml" then
      return
   end

   if (eventId == "QsYs" or eventId == "QsNo") and parameter1 == Question.takeJob then
      answerJob(eventId == "QsYs")
   end

   if eventId == "LpLL" and actorName == "siloLandingPad" and currentState == State.offered then
      c_askQuestion(Question.takeJob, "Changed your mind? Will you do it?", "Bruno")
   end

   if eventId == "OpTr" and parameter1 == Pad.lfLanded then
      startRepair(actorName)
   end

   if eventId == "ScTO" and parameter1 == repairTimer then
      finishRepair()
   end

   -- A timeout comes once per timer started since the last clear, so forget
   -- the id as soon as it has been handled
   if eventId == "ScTO" and parameter1 == reminderTimer then
      reminderTimer = -1
      c_addDialogMessage("Both are lit. Come to the Ore for your money.", "Bruno", true, 0, 0)
   end

   if eventId == "LpLL" and actorName == "oreLanding" and currentState == State.collect then
      collectReward()
   end
end

return mission2
