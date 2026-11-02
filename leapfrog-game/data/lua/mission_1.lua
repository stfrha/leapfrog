

local mission1 = {}

local swarm = require('swarm')

-- Mission 1's states, by name. currentState holds one of these numbers;
-- the number is what is saved, so give each state a number of its own and
-- don't renumber a state that saved games may hold. Always refer to a
-- state by its name: currentState == State.accepted.
local State = {
   -- Home scene
   offered          = 10,  -- Bruno has offered the job (asked when a new game
                           -- starts; again on landing at the silo or the Orb
                           -- until accepted)
   accepted         = 11,  -- fly to the Orb, one landing pad at a time;
                           -- Bruno has put free fuel in landingPad2 and 3
   atOrb            = 12,  -- Edith has cut in, the launch site is on the map

   -- The Debris Belt: the stolen stash (launched from the Orb's launch site)
   hunting          = 13,  -- clear the field; drops are his to keep
   stashIn          = 19,  -- nearly cleared: Bruno's rock drifts in from
                           -- the edge (stash_asteroid.xml)
   caseFound        = 14,  -- blasted open: a case inside (stash_case.xml)
   jackpot          = 15,  -- the case is open, the pickups are out; the
                           -- raiders are on their way
   raided           = 16,  -- the raiders' strippers take it all back
   stripped         = 17,  -- left with fuel to limp home
   done             = 18,  -- home at the silo, Edith is gone; mission 2 starts

   -- The old test mission (landing_scene, deep_space_scene). The main
   -- menu's test entries start it with state 1.
   testLandOnPad    = 1,   -- land on landing_pad1
   testTakeOff      = 2,   -- take off from it, to the grotto
   testFreeSpring   = 3,   -- break the spring free
   testDestroyBomb  = 4,   -- now destroy the bomb
   testFindTreasure = 5,   -- in deep space: find the coin treasure
   testEnded        = 6,   -- failed (bomb too early) or completed
}
mission1.State = State

-- A new game starts here
mission1.firstState = State.offered

-- With the job comes fuel to reach the launch site: when the leapfrog
-- lands on the Orb, the taskgiver fills the tank up to what the game
-- started with (leapfrog property 7 = fuel), read when the new game starts.
local homeStartFuel = 30

-- The launch site's square on the map shows up once Edith has had her say.
-- The timer is started with her dialog and lasts about as long as it
-- takes to show (frames).
local homeEdithDialogTicks = 660
local homeLaunchSiteTimer = -1

-- ------------------------------------------------------------------------
-- The stolen stash (deep space). The stash asteroid looks like any big
-- asteroid; blasted, it reveals the raiders' case; shot open, the case
-- spills the jackpot. Then the raiders' strippers come and take it all
-- back. (Upgrades: when they exist, the case gives a taste of them too.)

local stash = {
   asteroid = "stashAsteroid",   -- stash_asteroid.xml
   case     = "stashCase",       -- stash_case.xml (what the asteroid spawns)
   x = 860, y = 240,             -- outside the belt's right edge
   speedX = -40,                 -- and drifting in
   field = "asteroidField",      -- debris_belt_scene.xml
   fieldLeft = 3,                -- it comes when this few rocks of the field
                                 -- are left (not counting their pieces)
   space = "debris_belt_scene.xml",
}
mission1.Stash = stash

local edithGuessTicks = 600      -- Edith's wrong guess, 10 s into the hunt
local caseCheckTicks = 10        -- the case appears on the update after the
                                 -- asteroid breaks: listen to it then
local jackpotTicks = 900         -- 15 s to grab the loot before the raid
local limpHomeFuel = 15          -- what the raiders leave in the tank
mission1.limpHomeFuel = limpHomeFuel

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
mission1.space = function()
   if currentState == State.atOrb or inStash(currentState) then
      return stash.space
   end
   return nil
end

-- mission_script.lua's missionHolds: mission 1 runs the strippers itself
-- from the launch until it is done (the world's swarm stays away while
-- the stash is hunted, and after the raid so he can limp home)
mission1.holds = function(what)
   if what == "swarm" then
      return currentState == State.atOrb or inStash(currentState)
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

-- Landed at the silo after the raid
local function homeWithNothing()
   currentState = State.done
   c_addDialogMessage("Kid?", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Gone.", "Leapfrog", false, 120, 0)
   c_addDialogMessage("Did Bruno know whose stash that was?", "Leapfrog", false, 150, 0)
   c_hudLog("Mission complete")

   -- The story goes on at the silo: Bruno's next job
   lua_startMission(2)
end


-- Question ids (c_askQuestion): missions use 1 to 99
local Question = {
   acceptJob = 1,      -- Bruno's offer
}
mission1.Question = Question
local offerPending = false     -- asked and not yet answered (in this scene:
                               -- unanswered questions end with it)

-- The way to the Orb: the landing pads Bruno put fuel in, then the Orb.
-- The next one flashes on the map. Object and map actor (body) names
-- from home_scene.xml.
local route = {
   { pad = "landingPad2", body = "siloHome" },
   { pad = "landingPad3", body = "siloHome" },
   { pad = "oreLanding",  body = "noName" },
}
local routeStep = 1            -- index in route of the next stop
local routePadFuel = 15        -- free fuel in each of Bruno's pads

-- c_addDialogMessage(message, sender, leftSide, preDelay, postDelay)
-- Others on the left, the pilot on the right. Delays are in frames.
local function playBrunoCall()
   c_addDialogMessage("Hey! I hear you are in bad shape. Low fuel, low ammo, low everything.", "Bruno", true, 60, 0)
   c_addDialogMessage("Your so-called ship is falling apart, eh? You probably need a paying job, right? Like, really bad.", "Bruno", true, 120, 0)
   c_addDialogMessage("No. I'm in good shape. Where did you hear that?", "Leapfrog", false, 290, 0)
   c_addDialogMessage("Grapevine. If you are so strong, I can give this little job to someone else.", "Bruno", true, 220, 0)
   c_addDialogMessage("Hold up, let's talk about it.", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Get into town, over to the Orb.", "Bruno", true, 90, 0)
   c_addDialogMessage("I don't have enough fuel to get there.", "Leapfrog", false, 200, 0)
   c_addDialogMessage("Ok, I'm gonna make it easy for you. I will put in some credits into a couple of landing pads along the way. They will give you enough fuel to get here.", "Bruno", true, 300, 0)
   -- A question opens when the leapfrog stands on a pad (c_askQuestion).
   -- Its text is said when its turn comes (after any question already
   -- open is answered); only in flight Bruno adds his "touch down" line.
   c_askQuestion(Question.acceptJob, "Do you want the job?", "Bruno", "Touch down on the closest pad and let's discuss it.")
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

-- Shows the route on the map: the next stop flashes, the others are hollow
local function showRoute()
   for i, stop in ipairs(route) do
      local state = 0                         -- hollow
      if currentState == State.accepted and i == routeStep then
         state = 2                            -- flashing slowly
      end
      c_setObjectMapState(stop.pad, stop.body, state)
   end
end

-- Bruno's credits in the pads: free fuel (property 4 = fuel service,
-- 2 = free; 3 = fuel supply)
local function fillRoutePads()
   for i, stop in ipairs(route) do
      if stop.pad ~= "oreLanding" then
         c_setObjectProperty(stop.pad, 3, routePadFuel)
         c_setObjectProperty(stop.pad, 4, 2)
      end
   end
end

local function listenToRoute()
   for i, stop in ipairs(route) do
      listen(stop.pad, "LpLL")
   end
end

-- Landed on the Orb's pad with the job: the taskgiver adds some fuel and
-- Edith cuts in
local function arriveAtOrb()
   currentState = State.atOrb
   showRoute()

   -- Fill up, but never take fuel away
   if c_getObjectProperty("leapfrog1", 7) < homeStartFuel then
      c_setObjectProperty("leapfrog1", 7, homeStartFuel)
      c_hudLog("Fuel from the taskgiver")
   end

   playEdithCutsIn()
   homeLaunchSiteTimer = c_startSceneTimer(homeEdithDialogTicks)
end

local function askAgain(atOrb)
   if offerPending then
      return
   end

   if atOrb then
      c_askQuestion(Question.acceptJob, "You came anyway. So, do you want the job?", "Bruno")
   else
      c_askQuestion(Question.acceptJob, "Changed your mind? Do you want the job?", "Bruno")
   end
   offerPending = true
end

local function answerMission1(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("I'm going to think about it.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Suite yourself. Land back here if you change your mind.", "Bruno", true, 120, 0)
      return
   end

   currentState = State.accepted
   routeStep = 1

   -- Answered on the Orb's pad (the question opens on whatever pad the
   -- leapfrog lands on first; landing pad property 1 = leapfrog landed)
   if c_getObjectProperty("oreLanding", 1) == 1 then
      -- Already here
      c_addDialogMessage("I take it.", "Leapfrog", false, 20, 0)
      arriveAtOrb()
      return
   end

   c_addDialogMessage("I take it. See you at the orb.", "Leapfrog", false, 20, 0)
   c_addDialogMessage("Just follow the markers on your map. They will take you there, one step at the time.", "Bruno", true, 120, 0)

   fillRoutePads()
   listenToRoute()
   showRoute()
end

-- Landed on a pad of the route
local function routeLanding(pad)
   if currentState ~= State.accepted then
      return
   end

   if pad == "oreLanding" then
      arriveAtOrb()
      return
   end

   -- Move on from this pad (pads may be taken out of order)
   for i, stop in ipairs(route) do
      if stop.pad == pad and i >= routeStep then
         routeStep = i + 1
         showRoute()
      end
   end
end

-- Event handlers of the home scene
local function registerHomeHandlers()
   -- Launching is the world's job; here it starts the hunt
   listen("launchSite1", "LsLL")

   -- Bruno asks again when landing at home or at the Orb (state 1); the
   -- route's pads and the Orb (state 2)
   listen("siloLandingPad", "LpLL")
   listenToRoute()
end


-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
mission1.setupMissionStateScene = function()

   -- Triggers and handlers were cleared by lua_setupMissionStateScene;
   -- message senders are set by lua/common.lua

   if currentScene == "home_scene.xml" then

      -- Saved before the states got their names (home used 1, 2, 3)
      if currentState == 1 then
         currentState = State.offered
      elseif currentState == 2 then
         currentState = State.accepted
      elseif currentState < State.offered then
         currentState = State.atOrb
      end

      registerHomeHandlers()

      -- A question doesn't outlive its scene: whatever was open is gone,
      -- Bruno asks again on the next landing at the silo or the Orb
      offerPending = false

      -- Launch site on the map only once Edith has had her say
      c_setObjectMapShown("launchSite1", "mainTank", currentState >= State.atOrb)

      if currentState == State.accepted then
         -- Reloaded on the way to the Orb: Bruno's fuel is in the pads again
         fillRoutePads()
      end
      showRoute()

      if newGameStarting then
         -- The mining world script has put the leapfrog above the silo
         homeStartFuel = c_getObjectProperty("leapfrog1", 7)
         playBrunoCall()
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

mission1.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then

      -- Answers to c_askQuestion come as QsYs / QsNo, with the question
      -- id as parameter
      if parameter1 == Question.acceptJob then
         if eventId == "QsYs" then
            answerMission1(true)
         elseif eventId == "QsNo" then
            answerMission1(false)
         end
      end

      if eventId == "LpLL" and actorName == "siloLandingPad" and currentState == State.stripped then
         homeWithNothing()
      end

      if eventId == "LpLL" then
         if currentState == State.offered then
            if actorName == "siloLandingPad" then
               askAgain(false)
            elseif actorName == "oreLanding" then
               askAgain(true)
            end
         else
            routeLanding(actorName)
         end
      end

      if eventId == "ScTO" and parameter1 == homeLaunchSiteTimer then
         c_setObjectMapShown("launchSite1", "mainTank", true)
      end

      if eventId == "LsLL" then
         if currentState == State.atOrb then
            -- Off to find the stash (saved with the scene exit)
            currentState = State.hunting
            c_addDialogMessage("Hurry, get into space!", "Edith", true, 0, 0)
         end
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
return mission1
