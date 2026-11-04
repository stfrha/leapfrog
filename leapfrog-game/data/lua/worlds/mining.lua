-- The mining world (the home planet): its scenes' pads, services and
-- repeatable jobs, whatever mission is running, also in free play.
-- mission_script.lua calls this for the scenes it maps to "mining", after
-- lua/common.lua and before the mission. See common.lua for the rules.

local mining = {}

local swarm = require('swarm')
local debt = require('debt')

-- ------------------------------------------------------------------------
-- Leg repair at home (the silo's pad). Leapfrog property 10 is the lowest
-- leg joint health, 0..1 (0 = a joint is limp); writing it raises every
-- joint to at least that value. A full repair costs credits (property 8).
-- Without enough credits the silo patches limp joints for free, so the
-- legs hold but stay weak.

-- Question ids (c_askQuestion): the mining world uses 100 to 199
local Question = {
   repairLegs = 100,
}
mining.Question = Question
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
      c_askQuestion(Question.repairLegs, "Leg actuators damaged. Full repair for " .. legRepairCost .. " credits?", "Silo")
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


-- ------------------------------------------------------------------------
-- Re-entry outcomes (lua_reentryOutcome, when the orbit scene exits).
-- Down off the site: a recovery crew picks the Leapfrog up and sets it
-- down on the site's pad (scene_navigator.lua), for a fee by the distance (a site's marker is 100
-- planet units wide); what the credits don't cover, Bruno lends.
-- Bounced off the atmosphere: back in deep space, where the strippers
-- hold off for a while. Burned up: the game goes back to the latest save.
-- What is said comes at the next scene's setup.

local recoveryBaseFee = 20
local recoveryFeePerUnit = 0.4      -- 40 credits per marker width off
local bounceGraceTicks = 600        -- 10 s before the swarm starts

local arrivalLines = {}             -- { sender, text } for the next setup
local arrivalHud = {}               -- HUD log lines for the next setup
local bouncedBack = false           -- arriving in deep space by a bounce

local function recoveryFee(distance)
   return math.ceil(recoveryBaseFee + recoveryFeePerUnit * distance)
end
mining.recoveryFee = recoveryFee

mining.reentryOutcome = function(r)
   if r.outcome == "offSite" then
      local fee = recoveryFee(r.distance)
      arrivalLines[#arrivalLines + 1] = { "Recovery",
         "We picked you up out in the scrub and set you down on the pad. That's " .. fee .. " credits." }
      arrivalHud[#arrivalHud + 1] = "Recovery: " .. fee .. " credits"
      debt.charge(fee, "the recovery crew", true)

   elseif r.outcome == "bounced" then
      arrivalHud[#arrivalHud + 1] = "Bounced off the atmosphere"
      bouncedBack = true

   elseif r.outcome == "burnedUp" then
      arrivalHud[#arrivalHud + 1] = "Burned up on re-entry. Back to the latest save."
   end
end

local function sayArrival()
   for _, line in ipairs(arrivalLines) do
      c_addDialogMessage(line[2], line[1], true, 30, 0)
   end
   for _, text in ipairs(arrivalHud) do
      c_hudLog(text, true)
   end
   arrivalLines = {}
   arrivalHud = {}
end

local swarmTimer = -1

local function startSwarm()
   swarm.start("swarm", "stripper_swarm.xml", function(name)
      c_hudLog("The strippers have taken everything", true)
   end)
end


-- ------------------------------------------------------------------------
-- The bad mine: a hidden landing site (badMine in
-- mining_planet_surface.xml, hidden="true"). The docking station in deep
-- space hands over its re-entry solution (the key item badMineSolution)
-- the first time the Leapfrog docks; from then on the orbit shows the
-- site. A demonstration of key items and hidden sites, in every mission
-- and in free play.

local BadMine = {
   site = "badMine",
   solution = "badMineSolution",
   station = "dockingStation1",
}
mining.BadMine = BadMine

-- The site that isn't home (mining_planet_surface.xml)
local OtherSite = "goldMine"

local function dockedAtStation()
   -- Once: not again while held, and not if it was taken away
   if c_itemState(BadMine.solution) ~= "unknown" then
      return
   end

   c_addDialogMessage("Welcome aboard. A prospector left this with us before he went quiet: "
      .. "a re-entry solution for an old mine nobody works any more.", "Station", true, 30, 0)
   c_addDialogMessage("Take it. Watch yourself down there.", "Station", true, 90, 0)
   c_giveItem(BadMine.solution)
end


-- ------------------------------------------------------------------------
-- The beacons on the way from the silo into town, and past it. Mission 1
-- lights them. Holding villageSolution means all are lit, and then each
-- scene setup puts free fuel in their pads again, so they are stepping
-- stones between the silo and the town from then on. "lamp" is the sprite
-- with its blinking lamp; "base" a sprite under it to stop on its dark frame.

local Pad = {
   active      = 2,
   fuelSupply  = 3,
   fuelService = 4,    -- 2 = free fuel
}

local beaconFuel = 15          -- free fuel in a lit beacon's pad
mining.beaconFuel = beaconFuel

mining.villageSolution = "villagePadSolution"

mining.beacons = {
   { pad = "landingPad2", body = "siloHome", lamp = "landingPad2Lamp", period = 1.2, offFrame = 1 },
   { pad = "landingPad3", body = "siloHome", lamp = "landingPad3Lamp", period = 1.2, offFrame = 1 },
   { pad = "villagePad",  body = "siloHome", lamp = "villagePadLamp",  period = 1.25, offFrame = 4,
     base = "villagePadBase" },
}

-- gen_landing_pad_blink: 0 on, 1 off
local baseOffFrame = 1

local function stopBase(beacon)
   if beacon.base ~= nil then
      c_animateSprite(beacon.base, 0, baseOffFrame)
   end
end

mining.lightBeacon = function(beacon)
   stopBase(beacon)
   c_setObjectProperty(beacon.pad, Pad.active, 1)
   c_setObjectProperty(beacon.pad, Pad.fuelSupply, beaconFuel)
   c_setObjectProperty(beacon.pad, Pad.fuelService, 2)
   c_animateSprite(beacon.lamp, beacon.period)
   c_setObjectMapState(beacon.pad, beacon.body, 1)          -- filled
end

-- isNext: the one to land on, flashing on the map
mining.darkenBeacon = function(beacon, isNext)
   stopBase(beacon)
   c_setObjectProperty(beacon.pad, Pad.active, 0)
   c_animateSprite(beacon.lamp, 0, beacon.offFrame)
   c_setObjectMapState(beacon.pad, beacon.body, isNext and 2 or 0)  -- flashing or hollow
end

-- Which pad a re-entry at this scene ends on when the mission doesn't say
-- (lua_arrivalPad): the village pad once it can be landed on. nil = the
-- scene's default.
mining.arrivalPad = function(scene)
   if scene == "home_scene.xml" and c_hasItem(mining.villageSolution) then
      return "villagePad"
   end
   return nil
end


-- ------------------------------------------------------------------------
-- The shield power unit on the end of the station's boom, just outside the
-- station's shield (shield_power.xml, powerBoom in docking_station.xml).
-- In the big space all the time for now, to try it; mission 3 will decide.

local ShieldPower = {
   object = "shieldPower1",
   body = "shieldPower1.shieldPowerBody",
   x = 1010, y = 339.25,                   -- where the weld below puts it
   boom = "dockingStation1.powerBoom",
   boomAnchor = { x = -8.5, y = 0 },       -- the boom's tip
   unitAnchor = { x = 13, y = 0 },         -- the connector on the right cap
}
mining.ShieldPower = ShieldPower

local function addShieldPower()
   local p = ShieldPower
   if c_objectExists(p.object) then
      return
   end
   c_addPositionedChildObject("shield_power.xml", p.object, p.x, p.y)
   c_addWeld(p.boom, p.boomAnchor.x, p.boomAnchor.y,
      p.body, p.unitAnchor.x, p.unitAnchor.y)
end


-- ------------------------------------------------------------------------
-- Scene setup, before the mission's

mining.setupScene = function()

   sayArrival()

   if currentScene == "home_scene.xml" then

      -- Starting at home (a new game, or a saved game without a position):
      -- hover just above the silo's landing pad (the pad's top is at
      -- y = 588; in landing mode the leapfrog's origin is about 14 above
      -- its feet)
      if lfDefined == false and useInitialLfPos == true then
         c_addPositionedChildObject("leapfrog_landing.xml", "leapfrog1", 2736, 503)

         -- Reconfigure leapfrog to landing mode
         c_setObjectProperty("leapfrog1", 0, 2)
         lfDefined = true
      end

      -- The silo's pad is home: white on the map (landing pads are
      -- neutral by default). "siloHome" is the pad's box in home_scene.xml.
      c_setObjectMapAffiliation("siloLandingPad", "siloHome", "leapfrog")

      -- Landed at home: the silo offers to repair the legs
      listen("siloLandingPad", "LpLL")

      -- Dark until mission 1 says otherwise, lit for good after it
      local lit = c_hasItem(mining.villageSolution)
      for _, beacon in ipairs(mining.beacons) do
         if lit then
            mining.lightBeacon(beacon)
         else
            mining.darkenBeacon(beacon, false)
         end
      end
   end

   -- The launch site takes the Leapfrog to space, whatever mission is
   -- running, also in free play
   if currentScene == "home_scene.xml" or currentScene == "landing_scene.xml" then
      listen("launchSite1", "LsLL")
   end

   if currentScene == "orbit_scene.xml" then
      -- Known sites only: the bad mine once its solution is held
      c_setLandingSiteShown(BadMine.site, c_hasItem(BadMine.solution))

      -- A mission that ends at home has the planet to itself
      c_setLandingSiteShown(OtherSite, not missionHolds("landingSites"))

      if lua_arrivalPad("home_scene.xml") == "villagePad" then
         c_hudLog("Cleared for the village pad")
      end
   end

   if currentScene == "deep_space_scene.xml" then
      listen(BadMine.station, "DsDk")
      addShieldPower()
   end

   -- The strippers come whenever the Leapfrog is in space, unless the
   -- running mission has its own plans for them (mission 1's raid)
   -- Strippers disabled for now, to test the bad mine and the shield power unit without them interfering
   -- if currentSceneType == 1 and missionHolds("swarm") then
   if currentSceneType == 1  then
      bouncedBack = false
      swarmTimer = -1
   elseif currentSceneType == 1 then
      if bouncedBack or swarmTimer >= 0 then
         -- Back from a bounce: a breather before the swarm comes (again
         -- at a setup before it has started)
         bouncedBack = false
         swarmTimer = c_startSceneTimer(bounceGraceTicks)
      else
         startSwarm()
      end
   else
      swarmTimer = -1
   end

end


-- ------------------------------------------------------------------------
-- Events, before the mission's handler

mining.eventHandler = function(eventId, actorName, parameter1)

   if eventId == "ScTO" and swarmTimer >= 0 and parameter1 == swarmTimer then
      swarmTimer = -1
      startSwarm()
   end

   if currentScene == "home_scene.xml" then
      if actorName == "siloLandingPad" and eventId == "LpLL" then
         offerLegRepair()
      end
   end

   -- Nothing launches until a script says so (property 1)
   if eventId == "LsLL" then
      c_setObjectProperty(actorName, 1, 0)
   end

   if currentScene == "deep_space_scene.xml" then
      if actorName == BadMine.station and eventId == "DsDk" then
         dockedAtStation()
      end
   end

   -- Answers to c_askQuestion come as QsYs / QsNo, with the question id
   -- as parameter (asked on the silo's pad, so it opens at once, or after
   -- a question asked before it)
   if parameter1 == Question.repairLegs then
      if eventId == "QsYs" then
         answerLegRepair(true)
      elseif eventId == "QsNo" then
         answerLegRepair(false)
      end
   end
end

return mining
