

local mission1 = {}

-- Home scene, first mission: the taskgiver's call and Edith cutting in.
-- Edith speaks up when the leapfrog has landed on the Orb's pad
-- (oreLanding), so the dialog isn't read while flying.
local homeEdithDialogDone = false

-- The launch site's square on the map shows up once the mission has been
-- received, i.e. when Edith has had her say. The timer is started with
-- her dialog and lasts about as long as it takes to show (frames).
local homeEdithDialogTicks = 660
local homeLaunchSiteTimer = -1

-- With the job comes fuel to reach the launch site: when the leapfrog
-- lands on the Orb, the taskgiver fills the tank up to what the game
-- started with (leapfrog property 7 = fuel), read when the new game starts.
local homeStartFuel = 30

-- Leg repair at home (the silo's pad). Leapfrog property 10 is the lowest
-- leg joint health, 0..1 (0 = a joint is limp); writing it raises every
-- joint to at least that value.
-- A full repair costs credits (property 8). Without enough credits the
-- silo patches the joints for free, so the legs hold but stay weak.
local questionRepairLegs = 1   -- c_askQuestion id
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
   c_addDialogMessage("Ok, I'm gonna make it easy for you. I put in some credits into a couple of landing pads along the way. They will give you enough fuel to get here.", "Bruno", true, 90, 0)
   c_addDialogMessage("Just follow your markers on your map.", "Bruno", true, 190, 0)
   c_addDialogMessage("I see you there.", "Leapfrog", false, 200, 0)
   
end

local function playEdithCutsIn()
   c_addDialogMessage("I agree, you need that job.", "Edith", true, 0, 0)
   c_addDialogMessage("What!? Who is this?", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Nobody.", "Edith", true, 90, 0)
   c_addDialogMessage("This signal... it's coming from... you are in my home!", "Leapfrog", false, 90, 0)
   c_addDialogMessage("Wait till I get back there...", "Leapfrog", false, 60, 0)
   c_addDialogMessage("Not enough fuel. You'll be stranded.", "Edith", true, 90, 0)
end

-- Event handlers of the home scene, also set again after a trigger is
-- removed (c_clearAllTriggersAndEvents removes them all)
local function registerHomeHandlers()
   -- The launch site takes the leapfrog to deep space
   c_registerEventHandler("launchSite1", "LsLL", 0, 0)

   -- Landed on the Orb's pad: Edith cuts in (once)
   c_registerEventHandler("oreLanding", "LpLL", 0, 0)

   -- Landed at home: the silo offers to repair the legs
   c_registerEventHandler("siloLandingPad", "LpLL", 0, 0)
end


-- This script is related to one mission. It holds all specifics
-- for that mission in all states for all relevant scenes
-- It assumes the global variable currentScene to be set to 
-- the file name of the current scene.

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
mission1.setupMissionStateScene = function()

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()

   -- Who's who, for the colour of the message bubble frames. Senders not
   -- set here stay phosphor green. A message can also be given its own
   -- affiliation as a sixth argument to c_addDialogMessage.
   -- Set every scene setup, so it follows the mission state after a load
   -- (in Act 3: c_setSenderAffiliation("<pirate>", "friendly")).
   c_setSenderAffiliation("Edith", "leapfrog")
   c_setSenderAffiliation("Silo", "leapfrog")
   c_setSenderAffiliation("DEBUG", "leapfrog")
   c_setSenderAffiliation("Leapfrog", "leapfrog")
   c_setSenderAffiliation("Bruno", "neutral")

   -- Edith is on the Leapfrog's side, but her messages get a colour of
   -- their own so they don't look like the pilot's white ones
   c_setSenderColor("Edith", "ff80d5")

   if currentScene == "home_scene.xml" then

      registerHomeHandlers()

      -- Launch site on the map only after the mission has been received
      c_setObjectMapShown("launchSite1", "mainTank", homeEdithDialogDone)

      -- The silo's pad is home: white on the map (landing pads are
      -- neutral by default). "noName" is the pad's box in home_scene.xml.
      c_setObjectMapAffiliation("siloLandingPad", "siloHome", "leapfrog")

      if lfDefined == false then
         if useInitialLfPos == true then
            -- New game: start at home, hovering just above the silo's
            -- landing pad (the pad's top is at y = 588; in landing mode the
            -- leapfrog's origin is about 14 above its feet)
            c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 2736, 503)
            
            -- Reconfigure leapfrog to landing mode
            c_setObjectProperty("leapfrog1", 0, 2)
            lfDefined = true

            homeStartFuel = c_getObjectProperty("leapfrog1", 7)

            playBrunoCall()

            homeEdithDialogDone = false
         end
      end

   elseif currentScene == "landing_scene.xml" then

      -- Regardless of state, we always want to listen to leapfrog has landed 
      -- on the launch site
      c_registerEventHandler("launchSite1", "LsLL", 0, 0)

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
         c_registerEventHandler("landing_pad1", "LpLL", 0, 0)

         -- Detect if bomb exploded prematurely
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

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
         c_registerEventHandler("landing_pad1", "LpTO", 0, 0)
         
         -- Detect if bomb exploded prematurely
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

         -- Set state of next mission objective
         c_setObjectMapState("landing_pad1", "landingPad1", 0)
         c_setObjectMapState("coDestroyable", "springDestroyable", 1)

      elseif currentState == 3 then
         c_registerEventHandler("coDestroyable", "BrBR", 0, 0)
         c_registerEventHandler("bomb1", "ExEX", 0, 0)

      elseif currentState == 4 then
         c_registerEventHandler("bomb1", "ExEX", 0, 0)
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

         c_registerEventHandler("treasure", "PuPU", 0, 0)

      end
   end
end

mission1.missionStateSceneEventHandler = function(eventId, actorName, parameter1)

   if currentScene == "home_scene.xml" then

      if actorName == "siloLandingPad" and eventId == "LpLL" then
         offerLegRepair()
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

      if actorName == "oreLanding" and eventId == "LpLL" then
         -- Every landing there sends the event, so play only the first time
         if homeEdithDialogDone == false then
            homeEdithDialogDone = true

            -- The taskgiver adds some fuel; fill up, but never take fuel away
            if c_getObjectProperty("leapfrog1", 7) < homeStartFuel then
               c_setObjectProperty("leapfrog1", 7, homeStartFuel)
               c_hudLog("Fuel from the taskgiver")
            end

            playEdithCutsIn()
            homeLaunchSiteTimer = c_startSceneTimer(homeEdithDialogTicks)
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
