-- Mission "halfway": take the station back. The mission "stash" hands over
-- to it when the pilot lands at the silo, stripped. Bruno calls: the
-- Baxters have taken the docking station (Halfway), and there's a window
-- to take it back.
--
-- Up there the station's shield is red: it keeps the Leapfrog and its
-- bullets out. Its power unit sits on the end of the boom, outside the
-- bubble (shield_power.xml, welded on by mining.addShieldPower). Coming
-- close brings the Baxters' strippers, so he has to work fast: shoot the
-- unit off and the shield drops; dock, and hold on while the hack runs.
-- Undocking before it's done loses the hack. Once it's done the station is
-- his: its shield comes back friendly and throws the strippers off, the
-- Baxters call them back, and the station master hands over the bad mine's
-- re-entry solution, which the Baxters left behind. A while later the
-- Baxters pull out (placeholder lines, for Fredrik to write) and their
-- homestead leaves. Then free play, until the mission "badMine" exists.
--
-- Stripped bare before it's done: the strippers leave him enough fuel to
-- limp home, and he can try again from the launch site.

local mission = {}

local mining = require('mining')
local swarm = require('swarm')
local baxterShip = require('baxterShip')

-- Its states, by name. currentState holds one of these numbers; the number
-- is what is saved, so give each state a number of its own and don't
-- renumber a state that saved games may hold.
local State = {
   offered   = 10,  -- Bruno calls (again on landing at home until taken)
   toLaunch  = 11,  -- the launch site is on the map
   toStation = 12,  -- the station is theirs: red shield, power unit on
   powerDown = 13,  -- the unit is gone, the shield is down: dock to hack
   hacking   = 14,  -- docked, the hack runs
   liberated = 15,  -- the station is his; the Baxters are still about
   beaten    = 16,  -- stripped bare: home, and try again from the launch site
   done      = 17,  -- the Baxters have pulled out
}
mission.State = State

mission.firstState = State.offered

-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   takeBack = 1,
}
mission.Question = Question

local station = mining.Station
local power = mining.ShieldPower

-- Docking station properties
local StationProp = { active = 2, shield = 5 }
local Shield = { off = 0, friendly = 1, hostile = 2 }
mission.StationProp = StationProp
mission.Shield = Shield

-- Leapfrog properties
local xProperty = 3
local fuelProperty = 7

-- The strippers come when he comes this close (his x; the station is at
-- 1050, its power unit at about 1010, and launches arrive at x 20)
local approachX = 750
mission.approachX = approachX

-- The hack: docked this long, told in quarters
local hackTicks = 60 * 12
local hackSteps = 4
mission.hackTicks = hackTicks

-- From the hack to the Baxters pulling out
local pullOutTicks = 60 * 20
mission.pullOutTicks = pullOutTicks

-- What the strippers leave in the tank when he's stripped bare
local limpHomeFuel = 15
mission.limpHomeFuel = limpHomeFuel

local offerPads = { "siloLandingPad", "villagePad", "oreLanding" }
local offerPending = false

local approachTrigger = -1
local hackTimer = -1
local hackStep = 0
local pullOutTimer = -1

local function inSpaceTrip(state)
   return state == State.toStation or state == State.powerDown or
      state == State.hacking or state == State.liberated
end

-- mission_script.lua's missionHolds: no hammers or interceptors, the
-- strippers are the mission's, and the planet's other sites wait
mission.holds = function(what)
   if what == "enemies" or what == "swarm" then
      return currentState ~= State.done
   end
   if what == "landingSites" then
      return currentState == State.toLaunch or currentState == State.beaten or
         inSpaceTrip(currentState)
   end
   return false
end


-- ------------------------------------------------------------------------
-- Home: the call, the launch

local function playOffer()
   c_addDialogMessage("You still breathing? Good. Listen.", "Bruno", true, 300, 0)
   c_addDialogMessage("The Baxters have taken the station. Halfway. Shield up, red as a wound, and nobody docks.", "Bruno", true, 150, 0)
   c_addDialogMessage("Their shield runs off a power unit on the end of the boom. It's outside the bubble.", "Bruno", true, 180, 0)
   c_addDialogMessage("Knock it off, dock, and let the station's own system do the rest. You'll have to hold on while it does.", "Bruno", true, 180, 0)
   c_addDialogMessage("They won't be watching for long. Now's the time.", "Bruno", true, 180, 0)
   c_askQuestion(Question.takeBack, "Will you take the station back?", "Bruno")
   offerPending = true
end

local function askAgain()
   if offerPending then
      return
   end
   c_askQuestion(Question.takeBack, "The station's still theirs. Will you take it back?", "Bruno")
   offerPending = true
end

local function showLaunchSite()
   local going = currentState == State.toLaunch or currentState == State.beaten
   c_setObjectMapShown("launchSite1", "mainTank", going or currentState >= State.toLaunch)
   c_setObjectMapState("launchSite1", "mainTank", going and 2 or 0)
end

local function answerTakeBack(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("Not yet.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Don't wait too long. Land here when you're ready.", "Bruno", true, 120, 0)
      return
   end

   currentState = State.toLaunch
   c_addDialogMessage("I'll take it back.", "Leapfrog", false, 20, 0)
   c_addDialogMessage("That's the spirit. The launch site's on your map.", "Bruno", true, 120, 0)
   showLaunchSite()
end

local function launching()
   if currentState == State.beaten then
      c_addDialogMessage("Round two.", "Leapfrog", false, 60, 0)
   else
      c_addDialogMessage("Just me this time.", "Leapfrog", false, 60, 0)
   end
   currentState = State.toStation
end


-- ------------------------------------------------------------------------
-- Space: the shield, the unit, the hack

local function stationScene()
   return currentScene == station.scene
end

local function setShield(mode)
   c_setObjectProperty(station.object, StationProp.shield, mode)
end

local function startStrippers()
   if approachTrigger >= 0 then
      c_unregisterPropertyTrigger(approachTrigger)
      approachTrigger = -1
   end
   if swarm.isRunning("swarm") then
      return
   end
   swarm.start("swarm", "stripper_swarm.xml", function(name)
      mission.stripped()
   end)
end

local function watchApproach()
   approachTrigger = c_registerPropertyTrigger("leapfrog1", xProperty, "insideRange", approachX, 100000)
   listen("leapfrog1", "OpTr")
end

-- The station as the Baxters hold it
local function setupHeld()
   setShield(Shield.hostile)
   c_setObjectMapAffiliation(station.object, station.mapActor, "enemy")
   c_setObjectMapState(station.object, station.mapActor, 2)       -- flashing
   mining.addShieldPower()
   listen(power.object, "BrBR")
   baxterShip.show()
   c_hudLog("Sector: Halfway")
   c_addDialogMessage("There she is. Red shield. And that's their homestead parked behind her.", "Leapfrog", false, 90, 0)
   c_addDialogMessage("The unit on the end of the boom. Make it quick.", "Leapfrog", false, 150, 0)
   watchApproach()
end

-- The unit is gone: the shield is down, the station takes a docking
local function setupPowerDown()
   setShield(Shield.off)
   c_setObjectMapAffiliation(station.object, station.mapActor, "enemy")
   c_setObjectMapState(station.object, station.mapActor, 2)
   listen(station.object, "DsDk")
   listen(station.object, "DsUd")
   baxterShip.show()
end

local function powerBroken()
   currentState = State.powerDown
   setupPowerDown()
   startStrippers()
   c_hudLog("Station shield down", true)
   c_addDialogMessage("Shield's down. Now dock, and hold on.", "Leapfrog", false, 30, 0)
end

local function hackProgress()
   hackStep = hackStep + 1
   if hackStep < hackSteps then
      c_hudLog("Hacking: " .. math.floor(100 * hackStep / hackSteps) .. "%")
      hackTimer = c_startSceneTimer(math.floor(hackTicks / hackSteps))
      return
   end
   hackTimer = -1
   mission.liberate()
end

local function startHack()
   currentState = State.hacking
   hackStep = 0
   hackTimer = c_startSceneTimer(math.floor(hackTicks / hackSteps))
   c_hudLog("Hacking: 0%", true)
end

local function hackLost()
   currentState = State.powerDown
   hackTimer = -1
   c_hudLog("Hack lost: undocked", true)
   c_addDialogMessage("Lost it. Back in, quick!", "Leapfrog", false, 20, 0)
end

-- The station is his. Its master is still aboard; the Baxters kept him on
-- to run it.
local function stationMasterThanks()
   local master = mining.stationMaster
   c_addDialogMessage("Is it over? It's over! Welcome back to Halfway, friend.", master, true, 30, 0)
   c_addDialogMessage("The Baxters left a few things lying about. This was one: a re-entry solution for an old mine nobody works any more.", master, true, 150, 0)
   c_addDialogMessage("Take it. And as far as I'm concerned, I work for you now. The station's yours.", master, true, 180, 0)
   mining.giveBadMineSolution()
end

mission.liberate = function()
   currentState = State.liberated
   setShield(Shield.friendly)
   c_setObjectMapAffiliation(station.object, station.mapActor, "friendly")
   c_setObjectMapState(station.object, station.mapActor, 0)
   -- The Baxters call their strippers back
   if swarm.isRunning("swarm") then
      swarm.stop("swarm", true)
   end
   c_hudLog("Station hacked: Halfway is yours", true)
   stationMasterThanks()
   pullOutTimer = c_startSceneTimer(pullOutTicks)
end

-- PLACEHOLDER: the graffiti talk and Father Baxter's warning. Fredrik
-- writes these. "Baxter" (the son) and "Father Baxter" are red senders
-- (common.lua); add or remove lines as you like.
local pullOutLines = {
   -- { sender, text, preDelay }
   { "Baxter", "[Placeholder: the son on the graffiti on the station.]", 30 },
   { "Leapfrog", "[Placeholder: the pilot's answer.]", 150 },
   { "Father Baxter", "[Placeholder: Father Baxter's warning before they leave.]", 150 },
}
mission.pullOutLines = pullOutLines

local function pullOut()
   pullOutTimer = -1
   currentState = State.done
   for _, line in ipairs(pullOutLines) do
      c_addDialogMessage(line[2], line[1], line[1] ~= "Leapfrog", line[3], 0)
   end
   baxterShip.leave()
   -- The Baxters are gone from the world's space
   mining.setBaxtersInSpace(false)
   c_hudLog("Mission complete")
   -- Next: the bad mine (mission "badMine", not written yet). Free play
   -- until then.
   lua_startMission(freePlay)
end

-- Stripped bare before the station was his
mission.stripped = function()
   if currentState == State.liberated or currentState == State.done then
      return
   end
   currentState = State.beaten
   hackTimer = -1
   if c_getObjectProperty("leapfrog1", fuelProperty) < limpHomeFuel then
      c_setObjectProperty("leapfrog1", fuelProperty, limpHomeFuel)
   end
   c_addDialogMessage("That's everything. Go home, little frog.", "Raiders", true, 30, 0)
   c_hudLog("Stripped. Home, and try again.", true)
end

local function setupSpace()
   approachTrigger = -1
   hackTimer = -1
   pullOutTimer = -1

   if currentState == State.toStation then
      setupHeld()
   elseif currentState == State.powerDown or currentState == State.hacking then
      -- Back after leaving: the unit is gone, the shield is still down,
      -- and a hack that was running is lost
      currentState = State.powerDown
      setupPowerDown()
      startStrippers()
   elseif currentState == State.beaten then
      -- Stripped, on the way home: the station is still theirs
      setShield(Shield.hostile)
      c_setObjectMapAffiliation(station.object, station.mapActor, "enemy")
      baxterShip.show()
   elseif currentState == State.liberated then
      setShield(Shield.friendly)
      c_setObjectMapAffiliation(station.object, station.mapActor, "friendly")
      baxterShip.show()
      pullOutTimer = c_startSceneTimer(pullOutTicks)
   end
end


-- ------------------------------------------------------------------------
-- Scene setup and events

mission.setupMissionStateScene = function()

   if currentScene == "home_scene.xml" then

      offerPending = false
      showLaunchSite()
      for _, pad in ipairs(offerPads) do
         listen(pad, "LpLL")
      end

      if currentState == State.offered then
         playOffer()
      elseif currentState == State.beaten then
         c_addDialogMessage("Back in one piece, more or less. The station's still theirs. Go again when you're ready.", "Bruno", true, 60, 0)
      end

   elseif stationScene() then

      setupSpace()

   end
end

mission.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then

      if parameter1 == Question.takeBack then
         if eventId == "QsYs" then
            answerTakeBack(true)
         elseif eventId == "QsNo" then
            answerTakeBack(false)
         end
      end

      if eventId == "LpLL" and currentState == State.offered then
         askAgain()
      end

      -- Off to the station (saved with the scene exit). Going up before
      -- answering is taking the job.
      if eventId == "LsLL" and (currentState == State.offered or
         currentState == State.toLaunch or currentState == State.beaten) then
         if currentState == State.offered then
            c_cancelQuestion(Question.takeBack)
            offerPending = false
         end
         launching()
      end

   elseif stationScene() then

      if eventId == "OpTr" and parameter1 == approachTrigger then
         startStrippers()
      end

      if eventId == "BrBR" and actorName == power.object and currentState == State.toStation then
         powerBroken()
      end

      if eventId == "DsDk" and actorName == station.object and currentState == State.powerDown then
         startHack()
      end

      if eventId == "DsUd" and actorName == station.object and currentState == State.hacking then
         hackLost()
      end

      if eventId == "ScTO" and hackTimer >= 0 and parameter1 == hackTimer then
         if currentState == State.hacking then
            hackProgress()
         else
            hackTimer = -1
         end
      end

      if eventId == "ScTO" and pullOutTimer >= 0 and parameter1 == pullOutTimer then
         pullOut()
      end
   end
end

return mission
