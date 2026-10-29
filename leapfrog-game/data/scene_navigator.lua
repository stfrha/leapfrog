-- currentScene and gameProgress should be read from file to begin with
-- perhaps they are read from files and then sent as inputs to this script

currentScene = "New Game"
currentMission = 0

-- currentSceneType is: 0 = landing, 1 = deep space, 2 = orbit, 3 = hyperspace
currentSceneType = 0

gameProgress = 0

-- These variables are used in the mission_script to load the leapfrog at the 
-- correct position and configuration for the exit/entry combination. Thus, 
-- they are set here in the lua_sceneExitHandler function and then used in 
-- lua_setupMissionStateScene function. To use them, the useInitialConfiguration
-- must be set to false.
nextLfFile = "leapfrog_landing.xml"
nextLfPosX = 415
nextLfPosY = 657
nextLfMode = 2  -- 0 = Reset, 1 = deep space, 2 = Landing, 3 = Orbit, 4 == Reentry (What is the differnce between orbit and reentry?)
-- Or what is the difference between Deep space and orbit?

-- This is used to determine if the nextLfxxxxx variables declared above should
-- be used when the Leapfrog is created in the setupMissionStateScene function (false)
-- or if the exit-entry pair of the lua_sceneExitHandler function below
-- is used (true).
useInitialLfPos = false

lfDefined = false

-- This function starts a new or saved game. A new game is recognised by
-- its scene ("New Game" before anything is loaded, "not_defined" after
-- GameStatus::gameStatusNewGameInit); it starts mission 1 at home.
-- currentMission == 0 means no mission (free play), so it no longer means
-- a new game. For a saved game the variables currentSceneType,
-- currentState, currentScene and currentMission must be set using
-- lua_forceCurrentScene before this is called.
function lua_startInitialScene()

   lfDefined = false

   local newGame = (currentScene == "New Game") or (currentScene == "not_defined")

   if newGame or currentMission == 1 then
      currentScene = "home_scene.xml"
      currentSceneType = 0
      currentMission = 1
      currentState = lua_missionFirstState(currentMission)
      newGameStarting = true

      -- Start in the deep_space scene for test purposes. 
      -- currentScene = "deep_space_scene.xml"
      -- currentSceneType = 1
      -- currentMission = 1
      -- currentState = 5
   end

   useInitialLfPos = true -- For initial start, the LF file, pos and prop data is set by the setupMissionStateScene    
   
   -- Save the scene that was just determined. So we restart to it at later re-starts.
   c_saveGameStatus(currentScene, currentMission, currentState, currentSceneType)

   c_startScene(currentScene, currentSceneType)
end


-- exitSceneType is: 0 = landing, 1 = deep space, 2 = orbit, 3 = hyperspace
-- exitParameter is additional information related to the exit scene type

-- For exiting landing scene:
-- Parameter int is "leaving how":
-- 1 = deep space
-- Transition between ground-scenes are triggered by triggered events

-- For exiting deep space scene:
-- Parameter int is "leaving how":
-- 1 = upper right
-- 2 = upper left
-- 3 = lower right
-- 4 = lower left

-- For exiting orbit scene
-- Parameter int is "index of landing sites": the closest site when down on
-- the surface, on its marker or off it (c_lastReentry says which, and how
-- far off). lua_sceneExitHandler saves an off-site landing as
-- recoveredOffset + the index, so a continue puts the Leapfrog back on the
-- pad the recovery crew set it down on. Otherwise:
-- -1 = Down, but the planet has no landing sites
-- -2 = Burned up during re-entry (too much braking): back to the latest save
-- -3 = Bounced back to deep space (too little braking)
-- -4 = No burn while the sites came by: the orbit starts over (as entered
--      from deep space; nothing saved)

-- Exiting hyperspace scene is TBD

-- Arriving from orbit, per landing scene (the scene a site leads to, see
-- <landingSite scene="..."> in orbit_scene.xml): from above, in re-entry
-- configuration, after a good re-entry (you land on the pad yourself), or
-- standing on the pad the recovery crew set the Leapfrog down on after an
-- off-site landing (the pad's top less the ~14 from the Leapfrog's origin
-- to its feet in landing mode).
-- (Stand-ins for now: the gold mine site is home_scene with the silo's
-- pad, Alpha City is landing_scene with landing_pad1.)
local orbitArrivals = {
   ["home_scene.xml"]    = { from = { x = 2736, y = 50 }, pad = "siloLandingPad", onPad = { x = 2736, y = 500 } },
   ["landing_scene.xml"] = { from = { x = 520,  y = 50 }, pad = "landing_pad1",   onPad = { x = 415,  y = 657 } },
}

-- Added to the site index for an off-site landing (picked up and set down
-- on the site's pad by the recovery crew)
recoveredOffset = 100

function lua_findLeapfrogEntryPosition(exitSceneType, exitParameter)

   if exitSceneType == 0 and exitParameter == 1 then
      -- We are leaving landing scene to the deep_space_scene
      if currentScene == "deep_space_scene.xml" then
         nextLfFile = "leapfrog_deep_space.xml"
         nextLfPosX = 20
         nextLfPosY = 730
         nextLfMode = 1
         useInitialLfPos = false
         lfDefined = false
      end
   
   elseif exitSceneType == 1 and exitParameter == 3 then
      -- We are leaving deep space scene in lower right corner, to orbit scene
      if currentScene == "orbit_scene.xml" then
         nextLfFile = "leapfrog_deep_space.xml"
         nextLfPosX = -8
         nextLfPosY = 18
         nextLfMode = 1
         useInitialLfPos = false
         lfDefined = false
      end

   elseif exitSceneType == 2 and exitParameter == -3 then
      -- Bounced off the atmosphere: back in deep space near where the
      -- orbit was entered (the lower right corner), but out of its transit
      -- field (the insert burn pushes the Leapfrog on for a second)
      if currentScene == "deep_space_scene.xml" then
         nextLfFile = "leapfrog_deep_space.xml"
         nextLfPosX = 1050
         nextLfPosY = 640
         nextLfMode = 1
         useInitialLfPos = false
         lfDefined = false
      end

   elseif exitSceneType == 2 and exitParameter >= 0 then
      -- We are leaving orbit scene after a landing: currentScene is the
      -- site's landing scene; on the site (index) from above, recovered
      -- (recoveredOffset + index) standing on its pad
      local arrival = orbitArrivals[currentScene]
      if arrival ~= nil then
         if exitParameter >= recoveredOffset then
            nextLfFile = "leapfrog_landing.xml"
            nextLfPosX = arrival.onPad.x
            nextLfPosY = arrival.onPad.y
         else
            nextLfFile = "leapfrog_reentry.xml"
            nextLfPosX = arrival.from.x
            nextLfPosY = arrival.from.y
         end
         nextLfMode = 2
         useInitialLfPos = false
         lfDefined = false
      end
   end
end   

function lua_sceneExitHandler(exitSceneType, exitParameter)

   local oldScene = currentScene

   if exitSceneType == 0 then
      -- We are leaving landing scene
      -- Determine next scene
      if currentScene == "landing_scene.xml" or currentScene == "home_scene.xml" then
         if exitParameter == 1 then
            currentScene = "deep_space_scene.xml"
            currentSceneType = 1
            
            lua_findLeapfrogEntryPosition(exitSceneType, exitParameter)
            c_exitSceneSaveGameStatus(currentScene, currentMission, currentState, currentSceneType, oldScene, exitSceneType, exitParameter)
         end
      end
   
   elseif exitSceneType == 1 then
      -- We are leaving deep space scene
      -- Determine next scene
      if currentScene == "deep_space_scene.xml" then
         if exitParameter == 3 then
            currentScene = "orbit_scene.xml"
            currentSceneType = 2
            
            lua_findLeapfrogEntryPosition(exitSceneType, exitParameter)
            c_exitSceneSaveGameStatus(currentScene, currentMission, currentState, currentSceneType, oldScene, exitSceneType, exitParameter)
         end
      end
   
   elseif exitSceneType == 2 then
      -- We are leaving orbit scene
      -- Determine next scene
      if currentScene == "orbit_scene.xml" then
         local outcome, siteIndex, siteName, siteScene, distance, burn = c_lastReentry()

         -- The scripts price it first (a recovery fee, ...), so the save
         -- below has it
         lua_reentryOutcome({ outcome = outcome, siteIndex = siteIndex, siteName = siteName,
            siteScene = siteScene, distance = distance, burn = burn })

         if exitParameter == -2 then
            -- Burned up: dead, back to the latest save
            c_loadLatestSave()
            return
         elseif exitParameter == -4 then
            -- No site taken: round again, the orbit as entered from deep
            -- space (lower right corner). Not saved: the save is that
            -- arrival already.
            lua_findLeapfrogEntryPosition(1, 3)
            c_startScene(currentScene, currentSceneType)
            return
         elseif exitParameter == -3 then
            -- Bounced: deep space again, re-entry to try once more
            currentScene = "deep_space_scene.xml"
            currentSceneType = 1
         else
            -- Down, on or off a site: that site's landing scene
            if siteScene ~= nil and siteScene ~= "" then
               currentScene = siteScene
            else
               currentScene = "landing_scene.xml"
            end
            currentSceneType = 0

            -- Off the site: the recovery crew set it down on the pad
            if outcome == "offSite" and exitParameter >= 0 then
               exitParameter = recoveredOffset + exitParameter
            end
         end

         lua_findLeapfrogEntryPosition(exitSceneType, exitParameter)
         c_exitSceneSaveGameStatus(currentScene, currentMission, currentState, currentSceneType, oldScene, exitSceneType, exitParameter)
      end
   end

   c_startScene(currentScene, currentSceneType)

end

function lua_forceCurrentScene(newCurrentScene, newMission, newState, newSceneType)
   currentScene = newCurrentScene;
   currentMission = newMission;
   currentState = newState;
   currentSceneType = newSceneType
end

-- ------------------------------------------------------------------------
-- Test starts (lua/test_starts.lua): the main menu's test row

local testStarts = require('test_starts')

function lua_testStartCount()
   return #testStarts.list
end

function lua_testStartLabel(i)
   local e = testStarts.list[i]
   if e == nil then
      return ""
   end
   return e.label
end

-- Sets the scripts up for test start i, as a real scene change into its
-- scene would: the scene, mission and state, and the Leapfrog's entry
-- (lua_findLeapfrogEntryPosition for the arrival, or the scene's initial
-- position without one). Returns true and the scene, scene type, mission,
-- state, exit scene, exit type and exit parameter for MainActor (which
-- arms the scene), or false and a message.
function lua_prepareTestStart(i)
   local e = testStarts.list[i]
   if e == nil then
      return false, "no test start " .. tostring(i)
   end

   lua_forceCurrentScene(e.scene, e.mission, e.state, e.sceneType)
   newGameStarting = false
   lfDefined = false

   if e.from == nil then
      useInitialLfPos = true
      return true, e.scene, e.sceneType, e.mission, e.state, "", -1, 0
   end

   -- The entry for this arrival sets nextLfFile; nothing set = the
   -- navigator doesn't know this arrival
   nextLfFile = nil
   lua_findLeapfrogEntryPosition(e.from.type, e.from.param)

   if nextLfFile == nil then
      nextLfFile = "leapfrog_landing.xml"
      return false, "test start '" .. e.label .. "': no entry position for " ..
         e.scene .. " from " .. e.from.scene
   end

   return true, e.scene, e.sceneType, e.mission, e.state, e.from.scene, e.from.type, e.from.param
end
