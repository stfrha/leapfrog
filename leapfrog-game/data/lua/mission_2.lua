

local mission2 = {}

local swarm = require('swarm')

-- Mission 2: the stolen stash. Mission 1 hands over to it on the village pad,
-- with Bruno's offer.
--
-- Its states, by name. currentState holds one of these numbers;
-- the number is what is saved, so give each state a number of its own and
-- don't renumber a state that saved games may hold. Always refer to a
-- state by its name: currentState == State.toLaunch.
local State = {
   -- Home scene
   offered          = 10,  -- Bruno has offered the job (on the village pad;
                           -- again on landing there, at the Ore or at the
                           -- silo until accepted)
   toLaunch         = 12,  -- the launch site is on the map

   -- The Debris Belt: the stolen stash
   hunting          = 13,  -- clear the field; drops are his to keep
   stashIn          = 19,  -- nearly cleared: Bruno's rock drifts in from
                           -- the edge (stash_asteroid.xml)
   caseFound        = 14,  -- blasted open: a case inside (stash_case.xml)
   jackpot          = 15,  -- the case is open, the pickups are out; the
                           -- raiders are on their way
   raided           = 16,  -- the raiders' strippers take it all back
   stripped         = 17,  -- left with fuel to limp home
   done             = 18,  -- home at the silo, Edith is gone; free play

   -- The old test mission (landing_scene, deep_space_scene). The main
   -- menu's test entries start it with state 1.
   testLandOnPad    = 1,   -- land on landing_pad1
   testTakeOff      = 2,   -- take off from it, to the grotto
   testFreeSpring   = 3,   -- break the spring free
   testDestroyBomb  = 4,   -- now destroy the bomb
   testFindTreasure = 5,   -- in deep space: find the coin treasure
   testEnded        = 6,   -- failed (bomb too early) or completed
}
mission2.State = State

-- A new game starts here
mission2.firstState = State.offered

-- ------------------------------------------------------------------------
-- The stolen stash (deep space). The stash asteroid looks like any big
-- asteroid; blasted, it reveals the raiders' case; shot open, the case
-- spills the jackpot. Then the raiders' strippers come and take it all
-- back. (Upgrades: when they exist, the case gives a taste of them too.)

local stash = {
   asteroid = "stashAsteroid",   -- stash_asteroid.xml
   case     = "stashCase",       -- stash_case.xml (what the asteroid spawns)
   x = 585, y = 180,             -- in the belt's right boundary: deeper and
                                 -- it pushes the rock in far too fast
   speedX = -40,                 -- and drifting in
   field = "asteroidField",      -- debris_belt_scene.xml
   fieldLeft = 3,                -- it comes when this few rocks of the field
                                 -- are left (not counting their pieces)
   space = "debris_belt_scene.xml",
}
mission2.Stash = stash

local edithGuessTicks = 600      -- Edith's wrong guess, 10 s into the hunt
local caseCheckTicks = 10        -- the case appears on the update after the
                                 -- asteroid breaks: listen to it then
local jackpotTicks = 900         -- 15 s to grab the loot before the raid
local limpHomeFuel = 15          -- what the raiders leave in the tank
mission2.limpHomeFuel = limpHomeFuel
local edithFuel = 25             -- what Edith leaves in it at the silo
mission2.edithFuel = edithFuel

local edithGuessTimer = -1
local caseTimer = -1
local raidTimer = -1
local fieldWatch = -1

local function inStash(state)
   return state == State.hunting or state == State.stashIn
      or state == State.caseFound or state == State.jackpot
      or state == State.raided or state == State.stripped
end

-- Scene exits only save from the raid's end on: a belt exit is shut until the
-- Leapfrog has been stripped. Until then, a reload is a new launch.
local function beltExitOpen()
   return currentState == State.stripped
end

-- mission_script.lua's lua_missionSpace: the stash is in the Debris Belt
mission2.space = function()
   if currentState == State.toLaunch or inStash(currentState) then
      return stash.space
   end
   return nil
end

-- mission_script.lua's lua_arrivalPad: the raid ends at home, whichever pad
-- he would land on otherwise
mission2.arrivalPad = function(scene)
   if scene == "home_scene.xml" and inStash(currentState) then
      return "siloLandingPad"
   end
   return nil
end

-- mission_script.lua's missionHolds: mission 2 runs the strippers itself
-- from the launch until it is done (the world's swarm stays away while
-- the stash is hunted, and after the raid so he can limp home). "landingSites":
-- only home is on the planet until then, so the raid ends at the silo.
mission2.holds = function(what)
   if what == "swarm" or what == "landingSites" then
      return currentState == State.toLaunch or inStash(currentState)
   end
   return false
end

local function addStashAsteroid()
   if not c_objectExists(stash.asteroid) then
      c_addPositionedChildObject("stash_asteroid.xml", stash.asteroid, stash.x, stash.y)
   end
   listen(stash.asteroid, "BrBR")
end

-- Nearly cleared: the big rock drifts in from outside the belt
local function stashComesIn()
   currentState = State.stashIn
   addStashAsteroid()
   c_setObjectVelocity(stash.asteroid, stash.speedX, 0)
   c_addDialogMessage("There it is, drifting in from the edge. That big one.", "Bruno", true, 20, 0)
end

local function stashAsteroidBroken()
   currentState = State.caseFound
   c_addDialogMessage("There's something inside it! A box!", "Edith", true, 20, 0)
   c_addDialogMessage("So that's what Bruno is after.", "Leapfrog", false, 90, 0)
   caseTimer = c_startSceneTimer(caseCheckTicks)
end

local function stashCaseOpened()
   currentState = State.jackpot
   c_addDialogMessage("Whoo! Look at all that!", "Edith", true, 20, 0)
   c_addDialogMessage("Now we're talking.", "Leapfrog", false, 80, 0)
   c_hudLog("Stash found")
   raidTimer = c_startSceneTimer(jackpotTicks)
end

local function strippedByRaiders()
   currentState = State.stripped
   c_setObjectProperty("leapfrog1", 7, limpHomeFuel)
   c_setTransitOpen(3, true)
   c_addDialogMessage("That's everything. We left you some fuel. Crawl home.", "Raiders", true, 60, 0)
   c_hudLog("The raiders left " .. limpHomeFuel .. " fuel", true)
   c_hudLog("Transit clear", true)
end

-- The strippers come
local function startRaid()
   currentState = State.raided
   c_addDialogMessage("That's our stash you're grabbing, pilot.", "Raiders", true, 20, 0)
   c_addDialogMessage("We'll be taking it back. All of it.", "Raiders", true, 120, 0)
   c_addDialogMessage("Something's coming. From everywhere!", "Edith", true, 120, 0)
   swarm.start("swarm", "stripper_swarm.xml", function(name)
      strippedByRaiders()
   end)
end

local function setupStashHunt()
   c_setTransitOpen(3, beltExitOpen(), "Transit blocked: debris")

   -- The belt's exit stays shut until the raid is over, so everything before
   -- that starts over in a new field. (Saves from before the belt can hold
   -- any of these states.)
   if currentState ~= State.stripped then
      currentState = State.hunting
      c_hudLog("Sector: Debris Belt")
      c_addDialogMessage("Made it up? Good. Clear out some of this junk and the rock I want will drift out to you.", "Bruno", true, 60, 0)
      c_addDialogMessage("Whatever else you shake loose up there is yours to keep.", "Bruno", true, 150, 0)
      edithGuessTimer = c_startSceneTimer(edithGuessTicks)
      fieldWatch = c_watchFactory(stash.field, stash.fieldLeft)
   end
end

-- Landed at the silo after the raid. Edith has been and gone: what she
-- leaves in the tank gets him to the first beacon.
local function homeWithNothing()
   currentState = State.done
   c_addDialogMessage("Kid?", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Gone.", "Leapfrog", false, 120, 0)

   if c_getObjectProperty("leapfrog1", 7) < edithFuel then
      c_setObjectProperty("leapfrog1", 7, edithFuel)
   end
   c_addDialogMessage("A note on the console: \"Drained the old pump into your tank. You're welcome. Nobody.\"", "Silo", true, 150, 0)

   c_addDialogMessage("Did Bruno know whose stash that was?", "Leapfrog", false, 240, 0)
   c_hudLog("Mission complete")

   -- No next job yet
   lua_startMission(0)
end


-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   acceptJob = 1,      -- Bruno's offer
}
mission2.Question = Question
local offerPending = false     -- asked and not yet answered (in this scene:
                               -- unanswered questions end with it)

-- The Ore's and the silo's pads, and the one the job was offered on: Bruno
-- asks again there until the job is taken
local offerPads = { "villagePad", "oreLanding", "siloLandingPad" }

-- c_addDialogMessage(message, sender, leftSide, preDelay, postDelay)
-- Others on the left, the pilot on the right. Delays are in frames.
local function playOffer()
   c_addDialogMessage("Now for the real job. There's a rock of mine out in the Debris Belt.", "Bruno", true, 60, 0)
   c_addDialogMessage("Bring out what's in it. Everything else you shake loose up there is yours.", "Bruno", true, 150, 0)
   c_askQuestion(Question.acceptJob, "Do you want the job?", "Bruno")
   offerPending = true
end

local function waitingForLaunch()
   c_addDialogMessage("Yeay! We are going to space", "Edith", true, 120, 0)
   c_addDialogMessage("You sound young. How old are you?", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Nineteen", "Edith", true, 90, 0)
   c_addDialogMessage("Thirteen", "Edith", true, 150, 0)
   c_addDialogMessage("Well you stay away from my stuff.", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Brace for launch now", "Edith", true, 60, 0)
end

local function askAgain()
   if offerPending then
      return
   end
   c_askQuestion(Question.acceptJob, "Changed your mind? Do you want the job?", "Bruno")
   offerPending = true
end

local function showLaunchSite()
   c_setObjectMapShown("launchSite1", "mainTank", currentState >= State.toLaunch)
   c_setObjectMapState("launchSite1", "mainTank", 2)
end

local function answerMission2(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("I'm going to think about it.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Suit yourself. Land at the Ore if you change your mind.", "Bruno", true, 120, 0)
      return
   end

   currentState = State.toLaunch
   c_addDialogMessage("I take it.", "Leapfrog", false, 20, 0)
   c_addDialogMessage("Good. The launch site is on your map.", "Bruno", true, 120, 0)
   showLaunchSite()
end

-- Event handlers of the home scene
local function registerHomeHandlers()
   -- Launching is the world's job; here it starts the hunt
   listen("launchSite1", "LsLL")

   for _, pad in ipairs(offerPads) do
      listen(pad, "LpLL")
   end
end


-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
mission2.setupMissionStateScene = function()

   -- Triggers and handlers were cleared by lua_setupMissionStateScene;
   -- message senders are set by lua/common.lua

   if currentScene == "home_scene.xml" then

      -- The old test mission's states don't belong here
      if currentState < State.offered then
         currentState = State.toLaunch
      end

      registerHomeHandlers()

      -- A question doesn't outlive its scene: whatever was open is gone
      offerPending = false

      showLaunchSite()

      if currentState == State.offered then
         playOffer()
      end

   elseif currentScene == stash.space then

      setupStashHunt()

   elseif currentScene == "orbit_scene.xml" then

      if currentState == State.stripped then
         c_addDialogMessage("That went badly, huh?", "Edith", true, 120, 0)
      end

   elseif currentScene == "landing_scene.xml" then

      -- Regardless of state, we always want to listen to leapfrog has landed 
      -- on the launch site
      listen("launchSite1", "LsLL")

      if currentState == State.testLandOnPad then

         if lfDefined == false then 
            if useInitialLfPos == true then
               -- We will end up here for a new game and not during a exit/entry sequence
               -- Initialise leapfrog as descenting
               c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 527, 526)
               -- Reconfigure leapfrog to landing mode
               c_setObjectProperty("leapfrog1", 0, 2)
               lfDefined = true      
            end
         end
      
         c_addMissionStateSceneObjects("landing_scene_state2.xml")

         -- Start timer, will send ScTO event when timed out 
         state1Timer = c_startSceneTimer(240)

         -- Register event for landing on landingPad1
         listen("landing_pad1", "LpLL")

         -- Detect if bomb exploded prematurely
         listen("bomb1", "ExEX")

         -- Set bomb to be very in-sensitivity
         c_setObjectProperty("bomb1", 0, 250000)


         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 1)
         c_setObjectMapState("coDestroyable", "springDestroyable", 0)

         -- Play dialog
         c_addDialogMessage("Hello. Try to land on the landing pad, right below you.", "DEBUG", true, 0, 0)

      elseif currentState == State.testTakeOff then

         if lfDefined == false then 
            if useInitialLfPos == false then
               c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 415, 657)
               lfDefined = true      
            end
         end
           
         -- Register event for leaving (Take-off) landingPad1
         listen("landing_pad1", "LpTO")
         
         -- Detect if bomb exploded prematurely
         listen("bomb1", "ExEX")

         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 0)
         c_setObjectMapState("coDestroyable", "springDestroyable", 1)

      elseif currentState == State.testFreeSpring then
         listen("coDestroyable", "BrBR")
         listen("bomb1", "ExEX")

      elseif currentState == State.testDestroyBomb then
         listen("bomb1", "ExEX")
      end

   elseif currentScene == "deep_space_scene.xml" then

      if currentState == State.testFindTreasure then
      
         if lfDefined == false then 
            if useInitialLfPos == true then
               -- We will end up here for a new game and not during a exit/entry sequence
               -- Initialise leapfrog as descenting
               -- c_addPositionedChildObject("leapfrog_deep_space.xml", "leapfrog1", 20, 780)
               c_addPositionedChildObject("leapfrog_deep_space.xml", "leapfrog1", 20, 730)
               
               -- For test purposes, place leapfrog close to exit corner av  space scene.
               -- c_addPositionedChildObject("leapfrog_deep_space.xml", "leapfrog1", 1350, 680)            
            
               
               
               -- We dont want to change the mode since it makes leapfrog instable (and the mode is already correct)

               lfDefined = true      
            end
         end
      
         c_addPositionedChildObject("space_treasure.xml", "treasure", 700, 400)

         c_addDialogMessage("Welcome to space. Find the coin treasure.", "DEBUG", true, 0, 0)

         listen("treasure", "PuPU")

      end
   end
end

mission2.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then

      -- Answers to c_askQuestion come as QsYs / QsNo, with the question
      -- id as parameter
      if parameter1 == Question.acceptJob then
         if eventId == "QsYs" then
            answerMission2(true)
         elseif eventId == "QsNo" then
            answerMission2(false)
         end
      end

      if eventId == "LpLL" and actorName == "siloLandingPad" and currentState == State.stripped then
         homeWithNothing()
      end

      if eventId == "LpLL" and currentState == State.offered then
         askAgain()
      end

      if eventId == "LsLL" and currentState == State.toLaunch then
         -- Off to find the stash (saved with the scene exit)
         currentState = State.hunting
         waitingForLaunch()
      end

   elseif currentScene == stash.space and inStash(currentState) then

      if eventId == "FcLw" and parameter1 == fieldWatch then
         fieldWatch = -1
         stashComesIn()
      end

      if actorName == stash.asteroid and eventId == "BrBR" and currentState == State.stashIn then
         stashAsteroidBroken()
      end

      if eventId == "ScTO" and parameter1 == caseTimer then
         caseTimer = -1
         if c_objectExists(stash.case) then
            listen(stash.case, "BrBR")
         else
            caseTimer = c_startSceneTimer(caseCheckTicks)
         end
      end

      if actorName == stash.case and eventId == "BrBR" and currentState == State.caseFound then
         stashCaseOpened()
      end

      if eventId == "ScTO" and parameter1 == edithGuessTimer then
         edithGuessTimer = -1
         if currentState == State.hunting then
            c_addDialogMessage("Whoo, that one looks loaded!", "Edith", true, 0, 0)
         end
      end

      if eventId == "ScTO" and parameter1 == raidTimer then
         raidTimer = -1
         if currentState == State.jackpot then
            startRaid()
         end
      end

   elseif currentScene == "landing_scene.xml" then

      -- Regardless of state, we always want to listen to leapfrog has landed 
      -- on the launch site
      if eventId == "LsLL" then
         c_addDialogMessage("You will now be launched into deep space.", "DEBUG", true, 0, 0)
      end

      if currentState == State.testLandOnPad then
         if eventId == "ScTO" and parameter1 == state1Timer then

            -- Play dialog
            c_addDialogMessage("The landing pad is the filled square on your map. Get there!", "DEBUG", true, 0, 0)
            --c_addPositionedChildObject("medium_asteroid.xml", "dummy1", 500, 500)

         end

         if actorName == "landing_pad1" and eventId == "LpLL" then
            
            -- Play dialog
            c_addDialogMessage("Yippie: Welcome to the landing pad. Let me fill you up. A new obstacle has arrived in the grotto. Check it out.", "DEBUG", true, 0, 0)

            currentState = State.testTakeOff
            
            lua_setupMissionStateScene()
         end

         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = State.testEnded
            
            lua_setupMissionStateScene()
         end
      elseif currentState == State.testTakeOff then
         if actorName == "landing_pad1" and eventId == "LpTO" then
               
            -- Play dialog
            c_addDialogMessage("That's right. Get yourself over there.", "DEBUG", true, 0, 0)

            currentState = State.testFreeSpring

            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = State.testEnded
            
            lua_setupMissionStateScene()
         end

      elseif currentState == State.testFreeSpring then
         if actorName == "coDestroyable" and eventId == "BrBR" then
            -- Play dialog
            c_addDialogMessage("Good job, the spring is free. Now destroy the bomb.", "DEBUG", true, 0, 0)
            
            -- Set bomb to more sensitivity
            c_setObjectProperty("bomb1", 0, 2500)
            
            currentState = State.testDestroyBomb
            
            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = State.testEnded
            
            lua_setupMissionStateScene()
         end
      elseif currentState == State.testDestroyBomb then
         if actorName == "bomb1" then
            if eventId == "ExEX" then
               
               -- Play dialog
               c_addDialogMessage("Good work, you have earned a handsome reward. Ues it to get into space.", "DEBUG", true, 0, 0)

               currentState = State.testFindTreasure
               
               credits = c_getObjectProperty("leapfrog1", 8)
               c_setObjectProperty("leapfrog1", 8, credits + 200)

               -- Should not be done, since the scene exit/entry handling in c-code will
               -- do it.
               -- lua_setupMissionStateScene()
            end 
         end
      end
   elseif currentScene == "deep_space_scene.xml" then
      if currentState == State.testFindTreasure then
         if actorName == "treasure" then
            if eventId == "PuPU" then
               
               -- Play dialog
               c_addDialogMessage("Mission Complete, you are now a rich space pirate.", "DEBUG", true, 0, 0)

               currentState = State.testEnded

               lua_setupMissionStateScene()
            end 
         end
      end
   end
end

-- return { setupMissionStateScene = setupMissionStateScene, missionStateSceneEventHandler = missionStateSceneEventHandler}
return mission2
