-- Mission "package": a calm first trip to space. The mission "beacons" hands
-- over to it on the village pad. Bruno has something waiting for him at the
-- station; the pilot fetches it between jobs. Up there nothing hunts him: he
-- shoots rocks for what they shed, learns to dock (Edith talks him in), and
-- meets the station master, who is generous and warm and runs the station
-- for everybody. The station pays off, so the player cares when the Baxters
-- take it later (mission "halfway"). The package is sealed: nobody says
-- what's in it. Back on the village pad Bruno takes it, and from what was in
-- it he knows where the stash is: the mission "stash" starts there.

local mission = {}

local mining = require('mining')

-- Its states, by name. currentState holds one of these numbers; the number
-- is what is saved, so give each state a number of its own and don't
-- renumber a state that saved games may hold.
local State = {
   offered   = 10,  -- Bruno asks on the village pad (again on landing on
                    -- the village pad, at the Ore or at the silo until taken)
   toLaunch  = 11,  -- the launch site is on the map
   toStation = 12,  -- in space: rocks to shoot, the station to dock at
   carrying  = 13,  -- the sealed package aboard: down to the village pad
   done      = 14,
}
mission.State = State

mission.firstState = State.offered

-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   fetch = 1,
}
mission.Question = Question

local station = mining.Station        -- the docking station in the big space

-- Leapfrog properties
local ammoProperty = 5
local fuelProperty = 7
local creditsProperty = 8

-- The station master's welcome: a full tank and ammo for the way home, and
-- a little for the rocks cleared off his approach. Bruno pays on delivery.
local welcomeFuel = 60
local welcomeAmmo = 30
local rockMoney = 40
local deliveryPay = 80
mission.welcomeFuel = welcomeFuel
mission.welcomeAmmo = welcomeAmmo
mission.rockMoney = rockMoney
mission.deliveryPay = deliveryPay

local dockHintTicks = 60 * 45          -- Edith again, if not docked by then

local offerPads = { "villagePad", "oreLanding", "siloLandingPad" }
local offerPending = false
local dockHintTimer = -1

local function inSpaceTrip(state)
   return state == State.toStation or state == State.carrying
end

-- mission_script.lua's missionHolds: the station is the mission's, as it is
-- before anything happens to it (no bad mine handover, no power unit), and
-- nothing hunts the Leapfrog in space; the planet's other sites wait.
mission.holds = function(what)
   if what == "station" or what == "enemies" or what == "swarm" then
      return currentState ~= State.done
   end
   if what == "landingSites" then
      return currentState == State.toLaunch or inSpaceTrip(currentState)
   end
   return false
end

-- mission_script.lua's lua_arrivalPad: the package goes to the village pad
mission.arrivalPad = function(scene)
   if scene == "home_scene.xml" and currentState == State.carrying then
      return "villagePad"
   end
   return nil
end


-- ------------------------------------------------------------------------
-- Home: the offer, the launch, the delivery

local function pay(amount)
   local credits = c_getObjectProperty("leapfrog1", creditsProperty)
   c_setObjectProperty("leapfrog1", creditsProperty, credits + amount)
end

local function playOffer()
   c_addDialogMessage("Nice work on the beacons. Now, a small favour, when you get a chance. Between jobs.", "Bruno", true, 60, 0)
   c_addDialogMessage("There's a package waiting for me up at the station. Pick it up and bring it down here.", "Bruno", true, 150, 0)
   c_addDialogMessage("Shoot a few rocks on the way while you're up there. They shed all sorts of useful junk.", "Bruno", true, 150, 0)
   c_askQuestion(Question.fetch, "Will you fetch it?", "Bruno")
   offerPending = true
end

local function askAgain()
   if offerPending then
      return
   end
   c_askQuestion(Question.fetch, "That package is still waiting. Will you fetch it?", "Bruno")
   offerPending = true
end

local function showLaunchSite()
   c_setObjectMapShown("launchSite1", "mainTank", currentState >= State.toLaunch)
   c_setObjectMapState("launchSite1", "mainTank", 2)
end

local function answerFetch(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("Not right now.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("No rush. It's not going anywhere. Land here when you're ready.", "Bruno", true, 120, 0)
      return
   end

   currentState = State.toLaunch
   c_addDialogMessage("Sure, I'll pick it up.", "Leapfrog", false, 20, 0)
   c_addDialogMessage("Good. The launch site is on your map. Ask for the station master, he'll know.", "Bruno", true, 120, 0)
   showLaunchSite()
end

local function launching()
   currentState = State.toStation
   c_addDialogMessage("Space again! Can we stop at the station? Please say yes.", "Edith", true, 120, 0)
   c_addDialogMessage("We have to. That's the job.", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Yes!", "Edith", true, 60, 0)
end

-- Landed on the village pad with the package
local function deliver()
   currentState = State.done
   c_addDialogMessage("There it is. You didn't shake it, I hope.", "Bruno", true, 30, 0)
   c_addDialogMessage("It's sealed. I didn't touch it.", "Leapfrog", false, 120, 0)
   c_addDialogMessage("Of course you didn't. That's why I asked you.", "Bruno", true, 120, 0)
   pay(deliveryPay)
   c_hudLog("Mission complete: " .. deliveryPay .. " credits")
   -- What was in it tells Bruno where the stash is: he offers that job now
   lua_startMission("stash")
end

local function wrongPad(actorName)
   if actorName == "oreLanding" then
      c_addDialogMessage("Not here. The pad outside the village. Fewer eyes.", "Bruno", true, 30, 0)
   end
end


-- ------------------------------------------------------------------------
-- Space: rocks, the station, the station master

local function stationScene()
   return currentScene == station.scene
end

local function welcomeToSpace()
   c_hudLog("Sector: Halfway")
   c_addDialogMessage("Look, the station! Everybody stops there. Miners, haulers, everybody.", "Edith", true, 90, 0)
   c_addDialogMessage("I've read how docking works. Come in slow, nose first, right under the arm. The clamp does the rest.", "Edith", true, 180, 0)
   c_addDialogMessage("And shoot some rocks on the way. Free stuff!", "Edith", true, 180, 0)
end

local function dockHint()
   c_addDialogMessage("The station is the one flashing on your map. Slow and nose first, under the arm.", "Edith", true, 0, 0)
end

-- Docked for the first time on this trip: the station master. Generous and
-- warm, and the station is everybody's.
local function dockedAtStation()
   local master = mining.stationMaster
   currentState = State.carrying
   dockHintTimer = -1
   c_setObjectMapState(station.object, station.mapActor, 0)

   c_addDialogMessage("Welcome to Halfway, friend! Nobody fights inside my shield, and nobody leaves hungry.", master, true, 30, 0)
   c_addDialogMessage("Miners, haulers, drifters, the odd crook. They all dock here. It's everybody's station.", master, true, 150, 0)
   c_addDialogMessage("You cleared some rocks off my approach, too. That's worth a few credits, and a full tank on the house.", master, true, 160, 0)

   if c_getObjectProperty("leapfrog1", fuelProperty) < welcomeFuel then
      c_setObjectProperty("leapfrog1", fuelProperty, welcomeFuel)
   end
   c_setObjectProperty("leapfrog1", ammoProperty, c_getObjectProperty("leapfrog1", ammoProperty) + welcomeAmmo)
   pay(rockMoney)
   c_hudLog("Fuel and ammo from the station, " .. rockMoney .. " credits")

   c_addDialogMessage("I'm here for Bruno's package.", "Leapfrog", false, 150, 0)
   c_addDialogMessage("Ah. Here. Sealed, as he asked. He was very particular about that.", master, true, 90, 0)
   c_addDialogMessage("What's in it?", "Leapfrog", false, 120, 0)
   c_addDialogMessage("Didn't ask. Up here, that's good manners. Come back any time, and bring the kid.", master, true, 90, 0)
   c_addDialogMessage("He knows about me?!", "Edith", true, 150, 0)
   c_addDialogMessage("Everybody's heard you on the open channel, dear.", master, true, 90, 0)
   c_hudLog("Package aboard")
end

local function setupSpace()
   if currentState == State.toStation then
      listen(station.object, "DsDk")
      c_setObjectMapState(station.object, station.mapActor, 2)       -- flashing
      welcomeToSpace()
      dockHintTimer = c_startSceneTimer(dockHintTicks)
   end
end


-- ------------------------------------------------------------------------
-- Scene setup and events

mission.setupMissionStateScene = function()

   if currentScene == "home_scene.xml" then

      offerPending = false
      showLaunchSite()
      listen("launchSite1", "LsLL")
      for _, pad in ipairs(offerPads) do
         listen(pad, "LpLL")
      end

      if currentState == State.offered then
         playOffer()
      elseif currentState == State.carrying then
         c_setObjectMapState("villagePad", "siloHome", 2)                 -- flashing
      end

   elseif stationScene() then

      setupSpace()

   elseif currentScene == "orbit_scene.xml" then

      if currentState == State.carrying then
         c_addDialogMessage("What do you think is in it?", "Edith", true, 120, 0)
         c_addDialogMessage("Not our business.", "Leapfrog", false, 90, 0)
         c_addDialogMessage("It's ticking.", "Edith", true, 90, 0)
         c_addDialogMessage("What!?", "Leapfrog", false, 60, 0)
         c_addDialogMessage("Kidding.", "Edith", true, 60, 0)
      end
   end
end

mission.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then

      if parameter1 == Question.fetch then
         if eventId == "QsYs" then
            answerFetch(true)
         elseif eventId == "QsNo" then
            answerFetch(false)
         end
      end

      if eventId == "LpLL" and currentState == State.offered then
         askAgain()
      end

      -- Off to the station (saved with the scene exit). The world triggers
      -- the launch itself, also before the job is taken: going up is taking it.
      if eventId == "LsLL" and (currentState == State.toLaunch or currentState == State.offered) then
         if currentState == State.offered then
            c_cancelQuestion(Question.fetch)
            offerPending = false
         end
         launching()
      end

      if eventId == "LpLL" and currentState == State.carrying then
         if actorName == "villagePad" then
            deliver()
         else
            wrongPad(actorName)
         end
      end

   elseif stationScene() then

      if eventId == "DsDk" and actorName == station.object and currentState == State.toStation then
         dockedAtStation()
      end

      if eventId == "ScTO" and parameter1 == dockHintTimer then
         dockHintTimer = -1
         if currentState == State.toStation then
            dockHint()
         end
      end
   end
end

return mission
