

local mission1 = {}

-- Mission 1, home scene. currentState:
--   1 = Bruno has offered the job (asked when a new game starts; asked
--       again on landing at the silo or the Orb until accepted)
--   2 = accepted: fly to the Orb, one landing pad at a time; Bruno has
--       put free fuel in landingPad2 and landingPad3
--   3 = at the Orb: Edith has cut in, the launch site is on the map
-- (The landing_scene and deep_space_scene parts below are the old test
-- mission, states 1 to 6.)

-- With the job comes fuel to reach the launch site: when the leapfrog
-- lands on the Orb, the taskgiver fills the tank up to what the game
-- started with (leapfrog property 7 = fuel), read when the new game starts.
local homeStartFuel = 30

-- The launch site's square on the map shows up once Edith has had her say.
-- The timer is started with her dialog and lasts about as long as it
-- takes to show (frames).
local homeEdithDialogTicks = 660
local homeLaunchSiteTimer = -1

-- Bruno's offer (c_askQuestion id; the world uses 100 and up)
local acceptBrunoMission1 = 2
local offerPending = false     -- asked and not yet answered
local offerAtOrb = false       -- asked while standing on the Orb's pad

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
   c_addDialogMessage("Do you want the job?", "Bruno", true, 190, 0)
   c_askQuestion(acceptBrunoMission1)
   offerPending = true

end

local function playEdithCutsIn()
   c_addDialogMessage("I agree, you need that job.", "Edith", true, 0, 0)
   c_addDialogMessage("What!? Who is this?", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Nobody.", "Edith", true, 90, 0)
   c_addDialogMessage("This signal... it's coming from... you are in my home!", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Wait till I get back there...", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Not enough fuel. You'll be stranded.", "Edith", true, 90, 0)
end

-- Shows the route on the map: the next stop flashes, the others are hollow
local function showRoute()
   for i, stop in ipairs(route) do
      local state = 0                         -- hollow
      if currentState == 2 and i == routeStep then
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
   currentState = 3
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

   offerAtOrb = atOrb
   if atOrb then
      c_addDialogMessage("You came anyway. So, do you want the job?", "Bruno", true, 30, 0)
   else
      c_addDialogMessage("Changed your mind? Do you want the job?", "Bruno", true, 30, 0)
   end
   c_askQuestion(acceptBrunoMission1)
   offerPending = true
end

local function answerMission1(yes)
   offerPending = false

   if not yes then
      c_addDialogMessage("I'm going to think about it.", "Leapfrog", false, 20, 0)
      c_addDialogMessage("Suite yourself. Land back here if you change your mind.", "Bruno", true, 120, 0)
      return
   end

   currentState = 2
   routeStep = 1

   if offerAtOrb then
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
   if currentState ~= 2 then
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
   -- The launch site takes the leapfrog to deep space
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
   -- message senders are set by the world script

   if currentScene == "home_scene.xml" then

      registerHomeHandlers()

      -- Launch site on the map only once Edith has had her say
      c_setObjectMapShown("launchSite1", "mainTank", currentState >= 3)

      if currentState == 2 then
         -- Reloaded on the way to the Orb: Bruno's fuel is in the pads again
         fillRoutePads()
      end
      showRoute()

      if newGameStarting then
         -- The world script has put the leapfrog above the silo
         homeStartFuel = c_getObjectProperty("leapfrog1", 7)
         playBrunoCall()
      end

   elseif currentScene == "landing_scene.xml" then

      -- Regardless of state, we always want to listen to leapfrog has landed 
      -- on the launch site
      listen("launchSite1", "LsLL")

      if currentState == 1 then

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

      elseif currentState == 2 then

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

      elseif currentState == 3 then
         listen("coDestroyable", "BrBR")
         listen("bomb1", "ExEX")

      elseif currentState == 4 then
         listen("bomb1", "ExEX")
      end

   elseif currentScene == "deep_space_scene.xml" then

      if currentState == 5 then
      
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

      -- Answers to c_askQuestion come as QsYs / QsNo (QsWd: took off
      -- instead), with the question id as parameter
      if parameter1 == acceptBrunoMission1 then
         if eventId == "QsYs" then
            answerMission1(true)
         elseif eventId == "QsNo" then
            answerMission1(false)
         elseif eventId == "QsWd" then
            offerPending = false
         end
      end

      if eventId == "LpLL" then
         if currentState == 1 then
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
         c_addDialogMessage("You will now be launched into deep space.", "DEBUG", true, 0, 0)
         c_setObjectProperty("launchSite1", 1, 0)
      end

   elseif currentScene == "landing_scene.xml" then

      -- Regardless of state, we always want to listen to leapfrog has landed 
      -- on the launch site
      if eventId == "LsLL" then
         c_addDialogMessage("You will now be launched into deep space.", "DEBUG", true, 0, 0)
         c_setObjectProperty("launchSite1", 1, 0)
      end

      if currentState == 1 then
         if eventId == "ScTO" and parameter1 == state1Timer then

            -- Play dialog
            c_addDialogMessage("The landing pad is the filled square on your map. Get there!", "DEBUG", true, 0, 0)
            --c_addPositionedChildObject("medium_asteroid.xml", "dummy1", 500, 500)

         end

         if actorName == "landing_pad1" and eventId == "LpLL" then
            
            -- Play dialog
            c_addDialogMessage("Yippie: Welcome to the landing pad. Let me fill you up. A new obstacle has arrived in the grotto. Check it out.", "DEBUG", true, 0, 0)

            currentState = 2
            
            lua_setupMissionStateScene()
         end

         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 6
            
            lua_setupMissionStateScene()
         end
      elseif currentState == 2 then
         if actorName == "landing_pad1" and eventId == "LpTO" then
               
            -- Play dialog
            c_addDialogMessage("That's right. Get yourself over there.", "DEBUG", true, 0, 0)

            currentState = 3

            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 6
            
            lua_setupMissionStateScene()
         end

      elseif currentState == 3 then
         if actorName == "coDestroyable" and eventId == "BrBR" then
            -- Play dialog
            c_addDialogMessage("Good job, the spring is free. Now destroy the bomb.", "DEBUG", true, 0, 0)
            
            -- Set bomb to more sensitivity
            c_setObjectProperty("bomb1", 0, 2500)
            
            currentState = 4
            
            lua_setupMissionStateScene()
         end
         
         if actorName == "bomb1" and eventId == "ExEX" then
               
            -- Play dialog
            c_addDialogMessage("You exploded the bomb too early. Mission FAIL!!!", "DEBUG", true, 0, 0)

            currentState = 5
            
            lua_setupMissionStateScene()
         end
      elseif currentState == 4 then
         if actorName == "bomb1" then
            if eventId == "ExEX" then
               
               -- Play dialog
               c_addDialogMessage("Good work, you have earned a handsome reward. Ues it to get into space.", "DEBUG", true, 0, 0)

               currentState = 5
               
               credits = c_getObjectProperty("leapfrog1", 8)
               c_setObjectProperty("leapfrog1", 8, credits + 200)

               -- Should not be done, since the scene exit/entry handling in c-code will
               -- do it.
               -- lua_setupMissionStateScene()
            end 
         end
      end
   elseif currentScene == "deep_space_scene.xml" then
      if currentState == 5 then
         if actorName == "treasure" then
            if eventId == "PuPU" then
               
               -- Play dialog
               c_addDialogMessage("Mission Complete, you are now a rich space pirate.", "DEBUG", true, 0, 0)

               currentState = 6

               lua_setupMissionStateScene()
            end 
         end
      end
   end
end

-- return { setupMissionStateScene = setupMissionStateScene, missionStateSceneEventHandler = missionStateSceneEventHandler}
return mission1
