-- Mission 1: the dark beacons. The worked example in
-- documentation/lua_guide.md, so keep the guide in step with it (the mission
-- tester checks that the guide's code is still here).
--
-- A new game starts here. Bruno pays the pilot to relight the dead beacons on
-- the way into town, one at a time: two to the Ore, where he meets Bruno, then
-- the third, past the town. Mission 2 starts on the third beacon's pad.

local mission1 = {}

local mining = require('mining')

-- currentState is all that is saved of a mission, so every step the player
-- must not lose has a state of its own. The beacon states run in order: the
-- setup compares them with < and >.
local State = {
   offered      = 10,  -- Bruno has offered the job
   firstBeacon  = 11,  -- relight the first beacon
   secondBeacon = 12,
   toOre        = 13,  -- two lit, meet Bruno at the Ore
   thirdBeacon  = 14,  -- the one past the town
   done         = 15,
}
mission1.State = State

-- A new game or a test start begins here
mission1.firstState = State.offered

-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   takeJob = 2,
}
mission1.Question = Question

-- Each beacon of mining.beacons is lit in the state after its own
local beaconStates = { State.firstBeacon, State.secondBeacon, State.thirdBeacon }

-- Landing pad properties (see src/landingpad.h)
local lfLanded = 1

-- Leapfrog properties
local fuelProperty = 7
local creditsProperty = 8

local payAtOre = 100           -- credits
local payLastBeacon = 50
mission1.payAtOre = payAtOre
mission1.payLastBeacon = payLastBeacon

-- Bruno fills the tank up to this at the Ore: what the game started with
local startFuel = 30

local reminderTicks = 120      -- frames (60 a second)
local repairTicks = 180        -- a beacon takes 3 s to repair

local reminderTimer = -1       -- id of the running timer, -1 if none
local repairTimer = -1
local repairingPad = nil       -- the pad being repaired
local offerPending = false     -- asked and not yet answered (in this scene:
                               -- unanswered questions end with it)


-- ------------------------------------------------------------------------
-- Scene setup

-- Makes the beacons what currentState says. An inactive pad sends no LpLL,
-- so the next beacon is watched through its lfLanded property, which changes
-- active or not. "OpTr" arrives with the property id as parameter.
local function setupBeacons()
   for i, beacon in ipairs(mining.beacons) do
      local isNext = currentState == beaconStates[i]

      if currentState > beaconStates[i] then
         mining.lightBeacon(beacon)
      else
         mining.darkenBeacon(beacon, isNext)
      end

      if isNext then
         c_registerPropertyTrigger(beacon.pad, 0, lfLanded, "insideRange", 1, 1)
         listen(beacon.pad, "OpTr")
      end
   end
end

-- c_addDialogMessage(message, sender, leftSide, preDelay, postDelay)
-- Others on the left, the pilot on the right. Delays are in frames.
local function playBrunoCall()
   c_addDialogMessage("Hey! I hear you are in bad shape. Low fuel, low ammo, low everything.", "Bruno", true, 60, 0)
   c_addDialogMessage("Your so-called ship is falling apart, eh? You probably need a paying job, right? Like, really bad.", "Bruno", true, 120, 0)
   c_addDialogMessage("No. I'm in good shape. Where did you hear that?", "Leapfrog", false, 290, 0)
   c_addDialogMessage("Grapevine. If you are so strong, I can give this little job to someone else.", "Bruno", true, 220, 0)
   c_addDialogMessage("Hold up, let's talk about it.", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Get into town, over to the Ore.", "Bruno", true, 90, 0)
   c_addDialogMessage("I don't have enough fuel to get there.", "Leapfrog", false, 200, 0)
   c_addDialogMessage("The old beacons on the way have gone dark. Light them and their pads will fuel you up, and I'll pay for the work.", "Bruno", true, 300, 0)
   -- A question opens when the leapfrog stands on a pad (c_askQuestion).
   -- Its text is said when its turn comes (after any question already
   -- open is answered); only in flight Bruno adds his "touch down" line.
   c_askQuestion(Question.takeJob, "Do you want the job?", "Bruno", "Touch down on the closest pad and let's discuss it.")
   offerPending = true
end

local function playEdithCutsIn()
   c_addDialogMessage("I agree, you need that job.", "Edith", true, 0, 0)
   c_addDialogMessage("What!? Who is this?", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Nobody.", "Edith", true, 90, 0)
   c_addDialogMessage("This signal... it's coming from... you are in my home!", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Wait untill I get back there...", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Not enough fuel. You'll be stranded.", "Edith", true, 90, 0)
end

local function askAgain(atOre)
   if offerPending then
      return
   end

   if atOre then
      c_askQuestion(Question.takeJob, "You came anyway. So, do you want the job?", "Bruno")
   else
      c_askQuestion(Question.takeJob, "Changed your mind? Do you want the job?", "Bruno")
   end
   offerPending = true
end

-- Called for every new scene, and again by each change of state below.
-- Triggers and event listeners have been cleared, so everything that is
-- needed in this state is set up here, from currentState alone.
mission1.setupMissionStateScene = function()

   if currentScene ~= "home_scene.xml" then
      return
   end

   repairingPad = nil
   repairTimer = -1
   setupBeacons()

   -- Launching is the world's job; without the launch site on the map
   -- nothing leads there yet
   c_setObjectMapShown("launchSite1", "mainTank", false)

   if currentState == State.offered then
      listen("siloLandingPad", "LpLL")
      listen("oreLanding", "LpLL")
      offerPending = false

      if newGameStarting then
         -- The mining world script has put the leapfrog above the silo
         startFuel = c_getObjectProperty("leapfrog1", fuelProperty)
         playBrunoCall()
      else
         c_addDialogMessage("The old beacons have gone dark. I pay to have them lit.", "Bruno", true, 60, 0)
         c_askQuestion(Question.takeJob, "Do you want the job?", "Bruno", "Touch down and let's talk.")
         offerPending = true
      end

   elseif currentState == State.toOre then
      listen("oreLanding", "LpLL")
      c_setObjectMapState("oreLanding", "noName", 2)
      reminderTimer = c_startSceneTimer(reminderTicks)

   elseif currentState == State.firstBeacon or currentState == State.secondBeacon then
      -- Not there yet
      listen("oreLanding", "LpLL")
   end
end


-- ------------------------------------------------------------------------
-- Events

-- Goes to a new state and sets its scene up again
local function enterState(state)
   currentState = state
   lua_setupMissionStateScene()
end

local function pay(amount)
   local credits = c_getObjectProperty("leapfrog1", creditsProperty)
   c_setObjectProperty("leapfrog1", creditsProperty, credits + amount)
end

-- Fill up, but never take fuel away
local function topUp()
   if c_getObjectProperty("leapfrog1", fuelProperty) < startFuel then
      c_setObjectProperty("leapfrog1", fuelProperty, startFuel)
      c_hudLog("Fuel from Bruno")
   end
end

local function answerJob(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("Not now.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Suit yourself. Land at the silo if you change your mind.", "Bruno", true, 120, 0)
      return
   end

   c_addDialogMessage("I'll do it.", "Leapfrog", false, 20, 0)

   -- Answered on the Ore's pad (the question opens on whatever pad the
   -- leapfrog lands on first): he has come past the beacons on his own
   if c_getObjectProperty("oreLanding", lfLanded) == 1 then
      topUp()
      c_addDialogMessage("Then go back and light them, one at a time. They flash on your map.", "Bruno", true, 120, 0)
   else
      c_addDialogMessage("The next one flashes on your map. Land on it and stay put while it's repaired.", "Bruno", true, 120, 0)
   end

   enterState(State.firstBeacon)
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

-- The last beacon: the rest of the money, the solution for landing here, and
-- the next job on this pad
local function jobDone()
   currentState = State.done
   pay(payLastBeacon)
   c_giveItem(mining.villageSolution)
   c_hudLog("Mission complete: " .. payLastBeacon .. " credits")
   c_addDialogMessage("That's all of them. Here's the rest. Don't spend it all on fuel.", "Bruno", true, 30, 0)
   lua_startMission(2)
end

-- The repair is done if the Leapfrog stayed. The next setup lights the lamp
-- and shows the next beacon. Lighting the pad gives free fuel at once,
-- since the Leapfrog stands on it.
local function finishRepair()
   local padName = repairingPad
   repairingPad = nil
   repairTimer = -1

   if c_getObjectProperty(padName, lfLanded) ~= 1 then
      c_addDialogMessage("You left before it was done. Land there again and stay put.", "Bruno", true, 0, 0)
      return
   end

   c_hudLog("Beacon lit")

   if currentState == State.thirdBeacon then
      jobDone()
      return
   end

   if currentState == State.firstBeacon then
      playEdithCutsIn()
   end

   enterState(currentState + 1)
end

-- Bruno's money for the first two, his fuel, and the one past the town
local function meetBruno()
   pay(payAtOre)
   topUp()
   c_hudLog("Paid: " .. payAtOre .. " credits")
   c_addDialogMessage("Two down. Here's your money, the fuel station is next door.", "Bruno", true, 30, 0)
   c_addDialogMessage("One more, just past the town. Light that one and you get the rest.", "Bruno", true, 150, 0)
   c_setObjectMapState("oreLanding", "noName", 0)
   enterState(State.thirdBeacon)
end

mission1.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene ~= "home_scene.xml" then
      return
   end

   if (eventId == "QsYs" or eventId == "QsNo") and parameter1 == Question.takeJob then
      answerJob(eventId == "QsYs")
   end

   if eventId == "LpLL" and currentState == State.offered then
      askAgain(actorName == "oreLanding")
   end

   if eventId == "OpTr" and parameter1 == lfLanded then
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

   if eventId == "LpLL" and actorName == "oreLanding" then
      if currentState == State.toOre then
         meetBruno()
      elseif currentState == State.firstBeacon or currentState == State.secondBeacon then
         c_addDialogMessage("The beacons first. Then we talk.", "Bruno", true, 30, 0)
      end
   end
end

return mission1
