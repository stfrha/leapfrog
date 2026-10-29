-- The mock engine: stands in for the game's C++ side so the real mission
-- scripts can run headless. It mirrors, as closely as matters for mission
-- logic:
--   - LuaInterface: the c_* functions the scripts call, the modules the
--     game preloads (read from src/luainterface.cpp, so they stay in sync),
--     and the order the scripts are loaded in;
--   - MainActor: booting (new game / continue), starting scenes (c_startScene
--     arms a scene, which starts when the script returns), scene exits;
--   - GameStatus: what is in memory and what is in the save file;
--   - SceneActor: event listeners (c_registerEventHandler adds one per call,
--     so a double registration delivers an event twice, as in the game),
--     scene timers, the Yes/No questions (a queue; one opens while the
--     Leapfrog stands on a landing pad or launch site);
--   - CompoundObject: the scene's objects, read from the scene XML files, so
--     a misspelt object or map actor name is caught;
--   - the Leapfrog, landing pads and launch sites, as far as their
--     properties, events and fuel go. Physics is not simulated: a test says
--     what happens ("land on landingPad2") and the engine sends the events
--     the game would.
--
-- Anything the game would silently ignore (an unknown object, a map actor
-- that doesn't exist) or log as an error is recorded as a problem, and a
-- test fails on it unless it says it expects it.

local xml = require("testbench.xml")

local Engine = {}
Engine.__index = Engine

-- ------------------------------------------------------------------------
-- Construction and paths

function Engine.new(dataDir, srcDir)
   local self = setmetatable({}, Engine)
   self.dataDir = dataDir
   self.srcDir = srcDir
   self.file = nil            -- the save file (GameStatus XML), nil = none
   self.problems = {}         -- engine errors and warnings since last check
   self.log = {}              -- everything that happened, for failure reports
   self.xmlCache = {}
   self.listeners = {}
   self.questionQueue = {}
   self.timers = {}
   self.timerCounter = 0
   self.map = {}
   self:readInterface()
   self:resetGameStatus()
   return self
end

function Engine:path(file)
   return self.dataDir .. "/" .. file
end

function Engine:note(kind, text)
   self.log[#self.log + 1] = { kind = kind, text = text }
end

function Engine:problem(text)
   self.problems[#self.problems + 1] = text
   self:note("PROBLEM", text)
end

-- Reads src/luainterface.cpp for the modules the game preloads
-- (preloadModule("lua/x.lua", "name")) and the C functions it registers
-- (lua_register(m_L, "c_x", ...)), so the tester follows the game.
function Engine:readInterface()
   local f = io.open(self.srcDir .. "/luainterface.cpp", "rb")
   if not f then
      error("cannot read " .. self.srcDir .. "/luainterface.cpp (use --src)")
   end
   local text = f:read("a")
   f:close()

   self.modules = {}
   for file, name in text:gmatch('preloadModule%(%s*"([^"]+)"%s*,%s*"([^"]+)"%s*%)') do
      self.modules[#self.modules + 1] = { file = file, name = name }
   end

   self.registered = {}
   for name in text:gmatch('lua_register%(%s*m_L%s*,%s*"([^"]+)"') do
      self.registered[#self.registered + 1] = name
   end
end

-- ------------------------------------------------------------------------
-- GameStatus

local function legHealthDefaults()
   return { 1, 1, 1, 1, 1, 1 }
end

function Engine:resetGameStatus()
   self.status = {
      scene = "not_defined", mission = 0, state = 0, sceneType = 0,
      exitScene = "not_defined", exitType = -1, exitParam = -1,
      legs = legHealthDefaults(),
   }
   -- ObjectResources: shared with the Leapfrog's properties 5..9
   self.resources = { ammo = 1, shield = 1, fuel = 1, credits = 1, damage = 1 }
end

-- GameStatus::gameStatusNewGameInit (also writes the file)
function Engine:newGameInit()
   self.resources = { ammo = 70, shield = 100, fuel = 25, credits = 20, damage = 0 }
   local s = self.status
   s.scene, s.exitScene = "not_defined", "not_defined"
   s.sceneType, s.mission, s.state = 0, 0, 0
   s.exitType, s.exitParam = -1, -1
   s.legs = legHealthDefaults()
   self:saveGameStatus()
end

-- GameStatus::saveGameStatus: memory (and the Leapfrog's resources) to file
function Engine:saveGameStatus()
   local s, r = self.status, self.resources
   self.file = {
      scene = s.scene, mission = s.mission, state = s.state, sceneType = s.sceneType,
      exitScene = s.exitScene, exitType = s.exitType, exitParam = s.exitParam,
      legs = { table.unpack(s.legs) },
      ammo = r.ammo, shield = r.shield, fuel = r.fuel, credits = r.credits, damage = r.damage,
   }
   self:note("SAVE", string.format("scene %s, mission %d, state %d, exit %d/%d",
      s.scene, s.mission, s.state, s.exitType, s.exitParam))
end

-- GameStatus::readGameStatus: file to memory (not the resources)
function Engine:readGameStatus()
   local f = self.file
   if not f then
      return
   end
   local s = self.status
   s.scene, s.mission, s.state, s.sceneType = f.scene, f.mission, f.state, f.sceneType
   s.exitScene, s.exitType, s.exitParam = f.exitScene, f.exitType, f.exitParam
   s.legs = { table.unpack(f.legs) }
end

-- GameStatus::restoreGameStatus
function Engine:restoreGameStatus()
   if self.file then
      self:readGameStatus()
   else
      self:newGameInit()
   end
end

-- GameStatus::restoreLeapfrogResources (done when a Leapfrog is created)
function Engine:restoreLeapfrogResources()
   local f = self.file
   if f then
      self.resources = { ammo = f.ammo, shield = f.shield, fuel = f.fuel, credits = f.credits, damage = f.damage }
   else
      self.resources = { ammo = 1, shield = 1, fuel = 1, credits = 1, damage = 1 }
   end
end

-- ------------------------------------------------------------------------
-- XML files and objects

function Engine:readXml(file)
   if self.xmlCache[file] == nil then
      local root, err = xml.readFile(self:path(file))
      if not root then
         self:problem("cannot read " .. file .. ": " .. tostring(err))
         return nil
      end
      self.xmlCache[file] = root
   end
   return self.xmlCache[file]
end

local function isShapeTag(tag)
   return tag ~= "childObject" and tag ~= "behaviour" and tag ~= "system"
      and not tag:match("Joint$") and tag ~= "compoundObject"
end

-- An object from a <compoundObject> node: its shapes (map actors), its
-- behaviour and its own child objects
function Engine:objectFromNode(name, node)
   local obj = { name = name, actors = {}, children = {}, behaviour = nil, props = {}, triggers = {} }

   for _, c in ipairs(node.children) do
      if c.tag == "behaviour" then
         obj.behaviour = c.attr.type
         obj.behaviourAttr = c.attr
         obj.behaviourNode = c
      elseif c.tag == "childObject" then
         local child = self:objectFromChildNode(c)
         if child then
            obj.children[child.name] = child
         end
      elseif isShapeTag(c.tag) and c.attr.name then
         obj.actors[c.attr.name] = true
      end
   end

   self:initBehaviour(obj)
   return obj
end

-- A <childObject name=.. file=..> or <childObject name=..><compoundObject>
function Engine:objectFromChildNode(node)
   local name = node.attr.name
   if not name then
      return nil
   end
   if node.attr.file then
      local root = self:readXml(node.attr.file)
      if not root then
         return nil
      end
      return self:objectFromNode(name, root)
   end
   local co = xml.child(node, "compoundObject")
   if not co then
      return { name = name, actors = {}, children = {}, props = {}, triggers = {} }
   end
   return self:objectFromNode(name, co)
end

-- Property defaults per behaviour (see the classes' property lists)
function Engine:initBehaviour(obj)
   local a = obj.behaviourAttr or {}
   if obj.behaviour == "landingPad" then
      -- 0 state, 1 lfLanded (read only), 2 active, 3 fuelSupply, 4 fuelService
      local service = ({ none = 0, sell = 1, free = 2 })[a.fuel or "none"] or 0
      obj.props = { [0] = 2, [1] = 0, [2] = (a.active == "false") and 0 or 1,
         [3] = tonumber(a.fuelSupply or "100"), [4] = service }
      obj.fuelRate = tonumber(a.fuelRate or "12")
      obj.fuelPrice = tonumber(a.fuelPrice or "5")
   elseif obj.behaviour == "launchSite" then
      -- 0 state, 1 launch trigger, 2 active, 3 abort launch (write; reads 0)
      obj.props = { [0] = 0, [1] = 0, [2] = (a.active == "false") and 0 or 1, [3] = 0 }
   elseif obj.behaviour == "swarm" then
      -- Swarm: 0 active, 1 rate, 2 maxAlive, 3 alive (read only),
      -- 4 spawned (read only), 5 recall (write 1; reads 0)
      local sp = {}
      for _, c in ipairs(obj.behaviourNode and obj.behaviourNode.children or {}) do
         if c.tag == "swarmProperties" then sp = c.attr end
      end
      local ramp = tonumber(sp.rampTime or "0") > 0
      local rate = tonumber(sp.rate or "0.5")
      local maxAlive = tonumber(sp.maxAlive or "5")
      obj.props = { [0] = (sp.active == "false") and 0 or 1,
         [1] = ramp and tonumber(sp.startRate or rate) or rate,
         [2] = ramp and tonumber(sp.startMaxAlive or maxAlive) or maxAlive,
         [3] = 0, [4] = 0, [5] = 0 }
      obj.recalls = 0
   end
end

-- The scene's object by name; "a.b" looks into a's children, like
-- CompoundObject::getObject
function Engine:getObject(name)
   if not self.scene then
      return nil
   end
   local obj = nil
   local level = self.scene.objects
   for part in tostring(name):gmatch("[^%.]+") do
      obj = level[part]
      if not obj then
         return nil
      end
      level = obj.children
   end
   return obj
end

-- ------------------------------------------------------------------------
-- The Leapfrog's properties

local leapfrogResource = { [5] = "ammo", [6] = "shield", [7] = "fuel", [8] = "credits", [9] = "damage" }

function Engine:makeLeapfrog(name, file, x, y)
   local root = self:readXml(file)
   local obj = root and self:objectFromNode(name, root) or { name = name, actors = {}, children = {}, props = {}, triggers = {} }
   obj.isLeapfrog = true
   obj.file = file
   obj.x, obj.y = x, y
   obj.props = { [0] = 0, [1] = 0 }
   -- LeapFrog::initObjectResources: resources come from the save file
   self:restoreLeapfrogResources()
   return obj
end

function Engine:lowestLegHealth()
   local low = 1
   for _, h in ipairs(self.status.legs) do
      if h < low then low = h end
   end
   return low
end

function Engine:getProperty(obj, id)
   if obj.isLeapfrog then
      if leapfrogResource[id] then
         return self.resources[leapfrogResource[id]]
      elseif id == 10 then
         return self:lowestLegHealth()
      end
   end
   return obj.props[id] or 0
end

-- CompoundObject::extSetProperty, with the classes' own behaviour
function Engine:setProperty(obj, id, value)
   if obj.isLeapfrog then
      if leapfrogResource[id] then
         self.resources[leapfrogResource[id]] = value
      elseif id == 10 then
         -- Raises every leg joint to at least the value
         local v = math.max(0, math.min(1, value))
         for i, h in ipairs(self.status.legs) do
            if h < v then self.status.legs[i] = v end
         end
      else
         obj.props[id] = value
      end
   else
      obj.props[id] = value
      -- A launch site's property 1 is its launch trigger: setting it (to
      -- any value) starts the launch sequence
      if obj.behaviour == "launchSite" and id == 1 then
         obj.launchTriggered = true
      end
      -- Property 3 takes back a triggered launch that still waits for
      -- clearance (in the tester: any triggered launch not yet gone)
      if obj.behaviour == "launchSite" and id == 3 then
         obj.props[3] = 0
         if obj.launchTriggered then
            obj.launchTriggered = false
            obj.launchAborts = (obj.launchAborts or 0) + 1
            self:note("LAUNCH", obj.name .. ": launch aborted")
         end
      end
      -- A swarm's property 5 is recall: the live strippers leave; it reads 0
      if obj.behaviour == "swarm" and id == 5 and value ~= 0 then
         obj.recalls = obj.recalls + 1
         obj.props[5] = 0
      end
   end

   self:note("PROP", string.format("%s[%d] = %s", obj.name, id, tostring(value)))
   self:checkTriggers(obj, id)
end

-- Property triggers (c_registerPropertyTrigger), sent as "OpTr"
function Engine:checkTriggers(obj, id)
   for _, t in ipairs(obj.triggers) do
      if t.propertyId == id then
         local v = self:getProperty(obj, id)
         local hit = false
         if t.type == "insideRange" then
            hit = v >= t.lower and v <= t.upper
         elseif t.type == "outsideRange" then
            hit = v < t.lower or v > t.upper
         end
         if hit then
            self:deliver(obj.name, "OpTr", id)
         end
      end
   end
end

-- ------------------------------------------------------------------------
-- Lua: one environment per game process (the game creates its Lua state
-- once, at start; a new game from the menu reuses it)

function Engine:newLua()
   local engine = self
   local env = {}

   -- The standard library the game's Lua state has (luaL_openlibs)
   for _, k in ipairs({ "assert", "error", "ipairs", "next", "pairs", "pcall", "rawequal",
      "rawget", "rawlen", "rawset", "select", "setmetatable", "getmetatable", "tonumber",
      "tostring", "type", "xpcall", "string", "table", "math", "utf8", "coroutine", "os", "io" }) do
      env[k] = _G[k]
   end
   env._G = env
   env.print = function(...)
      local parts = {}
      for i = 1, select("#", ...) do parts[#parts + 1] = tostring((select(i, ...))) end
      engine:note("print", table.concat(parts, "\t"))
   end

   -- require with the game's preloaded modules
   local preload, loaded = {}, {}
   env.package = { preload = preload, loaded = loaded }
   env.require = function(name)
      if loaded[name] ~= nil then
         return loaded[name]
      end
      local loader = preload[name]
      if not loader then
         error("module '" .. tostring(name) .. "' not preloaded by the game", 2)
      end
      local result = loader(name)
      if result == nil then result = true end
      loaded[name] = result
      return result
   end

   self:installApi(env)

   self.env = env

   -- LuaInterface::initLuaInterface: preload the modules, then run
   -- scene_navigator.lua and mission_script.lua
   for _, m in ipairs(self.modules) do
      local chunk, err = self:loadFile(m.file, env)
      if chunk then
         preload[m.name] = chunk
      else
         self:problem("loading " .. m.file .. " failed: " .. err)
      end
   end

   for _, file in ipairs({ "scene_navigator.lua", "mission_script.lua" }) do
      local chunk, err = self:loadFile(file, env)
      if not chunk then
         self:problem("loading " .. file .. " failed: " .. err)
      else
         local ok, e = pcall(chunk)
         if not ok then
            self:problem("running " .. file .. " failed: " .. tostring(e))
         end
      end
   end
end

function Engine:loadFile(file, env)
   local f = io.open(self:path(file), "rb")
   if not f then
      return nil, "cannot open " .. self:path(file)
   end
   local text = f:read("a")
   f:close()
   return load(text, "@" .. file, "t", env)
end

-- Calls a global Lua function of the game, like the C++ side does
-- (lua_getglobal + lua_pcall: an error is logged and the game goes on)
function Engine:callLua(name, ...)
   local fn = self.env[name]
   if type(fn) ~= "function" then
      self:problem("the scripts have no function " .. name)
      return
   end
   local ok, err = pcall(fn, ...)
   if not ok then
      self:problem("Lua error in " .. name .. ": " .. tostring(err))
   end
end

-- As callLua, returning the function's results in a table (nil on error)
function Engine:callLuaResults(name, ...)
   local fn = self.env[name]
   if type(fn) ~= "function" then
      self:problem("the scripts have no function " .. name)
      return nil
   end
   local r = table.pack(pcall(fn, ...))
   if not r[1] then
      self:problem("Lua error in " .. name .. ": " .. tostring(r[2]))
      return nil
   end
   return { table.unpack(r, 2, r.n) }
end

-- ------------------------------------------------------------------------
-- Scenes

-- MainActor::armScene: the scene starts when the current script is done
function Engine:armScene(file, sceneType)
   self.pendingScene = { file = file, sceneType = sceneType }
   self:note("SCENE", "armed " .. tostring(file) .. " (type " .. tostring(sceneType) .. ")")
end

-- Starts armed scenes (MainActor::startScene)
function Engine:runPending()
   local guard = 0
   while self.pendingScene do
      guard = guard + 1
      if guard > 10 then
         self:problem("scenes keep starting scenes")
         return
      end
      local p = self.pendingScene
      self.pendingScene = nil
      self:startScene(p.file, p.sceneType)
   end
   self:updateQuestions()
end

-- SceneActor::updateQuestion: questions open one at a time, in the order
-- asked, while the Leapfrog stands on a landing pad or a launch site (not
-- yet launched). Leaving before an answer puts the open one back in front.
-- (The game also waits for the dialog to catch up and the booster and fire
-- buttons to be up before showing it; the tester shows it at once.)
function Engine:leapfrogStands()
   local obj = self.landedOn
   return obj ~= nil and (obj.behaviour == "landingPad" or obj.behaviour == "launchSite")
end

function Engine:updateQuestions()
   local queue = self.questionQueue or {}
   self.questionQueue = queue
   local stands = self:leapfrogStands()

   if self.question and not stands then
      table.insert(queue, 1, self.question)
      self.questionFlight[self.question] = nil
      self:note("QUESTION", string.format("%d back in the queue (not standing)", self.question))
      self.question = nil
   end

   -- In flight with the next question waiting: its in-flight line, once
   -- (in the game when the dialog has caught up; the tester's dialog is
   -- always caught up)
   if not self.question and not stands and #queue > 0 then
      local f = (self.questionFlight or {})[queue[1]]
      if f and not f.given then
         f.given = true
         self.dialog[#self.dialog + 1] = { sender = f.sender, text = f.text, left = true }
         self:note("DIALOG", f.sender .. ": " .. f.text .. " (question " .. queue[1] .. ", in flight)")
      end
   end

   if not self.question and stands and #queue > 0 then
      self.question = table.remove(queue, 1)
      self:note("QUESTION", string.format("%d open (on %s)", self.question, self.landedOn.name))
   end
end

function Engine:hasPendingQuestion()
   return self.question ~= nil or #(self.questionQueue or {}) > 0
end

function Engine:startScene(file, sceneType)
   -- clearAllTriggersAndEvents, then the old scene goes
   self.listeners = {}
   self.scene = nil
   self.question = nil        -- the open question (id), nil: none
   self.questionQueue = {}    -- asked, waiting for the Leapfrog to stand
   self.questionFlight = {}   -- id -> in-flight line { text, sender, given }
   self.landedOn = nil
   self.timers = {}
   self.timerCounter = 0
   self.map = {}

   local root = self:readXml(file)
   if not root then
      return
   end

   local scene = { file = file, sceneType = sceneType, objects = {} }
   for _, c in ipairs(root.children) do
      if c.tag == "childObject" then
         local obj = self:objectFromChildNode(c)
         if obj then
            scene.objects[obj.name] = obj
         end
      end
   end
   self.scene = scene
   self.sceneCount = (self.sceneCount or 0) + 1
   self:note("SCENE", "started " .. file)

   self:callLua("lua_setupMissionStateScene")

   -- The initial scene of a mission is saved once it is set up (not a
   -- test start's)
   if self.status.exitType == -1 and not self.startingTest then
      self:saveGameStatus()
   end
   self.startingTest = false
end

-- MainActor's boot: restore the save, then continue where it was
function Engine:boot()
   self:newLua()
   self:continueGame()
end

-- The menu's continue, and the boot after restoring (MainActor)
function Engine:continueGame()
   self:restoreGameStatus()
   if self.status.exitType == -1 then
      self:callLua("lua_startInitialScene")
   else
      self:forceCurrentScene()
      self:callLua("lua_findLeapfrogEntryPosition", self.status.exitType, self.status.exitParam)
      self:armScene(self.status.scene, self.status.sceneType)
   end
   self:runPending()
end

-- The menu's new game (same Lua state)
function Engine:menuNewGame()
   if not self.env then
      self:newLua()
   end
   self:resetGameStatus()
   self:newGameInit()
   self:forceCurrentScene()
   self:callLua("lua_startInitialScene")
   self:runPending()
end

-- The menu's test row (MainActor, PostMenuActionType::testStart): the
-- scripts prepare test start i (lua/test_starts.lua), the game status
-- follows, the scene is armed, and its start doesn't save the game.
-- Returns false and the message if the scripts refuse it.
function Engine:menuTestStart(i)
   if not self.env then
      -- The menu is there once the game has started
      self:newLua()
   end
   local r = self:callLuaResults("lua_prepareTestStart", i)
   if not r or not r[1] then
      local message = r and r[2] or "lua_prepareTestStart failed"
      self:note("TEST", "refused: " .. tostring(message))
      return false, message
   end
   local s = self.status
   s.scene, s.sceneType, s.mission, s.state = r[2], r[3], r[4], r[5]
   s.exitScene, s.exitType, s.exitParam = r[6], r[7], r[8]
   self.startingTest = true
   self:armScene(s.scene, s.sceneType)
   self:runPending()
   return true
end

-- The test starts the menu offers: count and labels
function Engine:testStarts()
   local count = self:callLuaResults("lua_testStartCount")
   local list = {}
   for i = 1, (count and count[1] or 0) do
      local label = self:callLuaResults("lua_testStartLabel", i)
      list[i] = label and label[1] or ""
   end
   return list
end

function Engine:forceCurrentScene()
   local s = self.status
   self:callLua("lua_forceCurrentScene", s.scene, s.mission, s.state, s.sceneType)
end

-- A scene exit (the scene's ExitXxxSceneEvent): Lua picks the next scene
function Engine:exitScene(sceneType, parameter)
   self:note("EXIT", string.format("scene type %d, parameter %d", sceneType, parameter))
   self:callLua("lua_sceneExitHandler", sceneType, parameter)
   self:runPending()
end

-- ------------------------------------------------------------------------
-- Events

function Engine:listenerCount(actor, eventId)
   return (self.listeners or {})[actor .. "/" .. eventId] or 0
end

-- Sends an event the way the game does: once per registered listener
-- (none, if nobody listens). Returns the number of deliveries.
function Engine:deliver(actor, eventId, parameter)
   local n = self:listenerCount(actor, eventId)
   self:note("EVENT", string.format("%s %s %s%s", eventId, actor, tostring(parameter or 0),
      n == 0 and " (nobody listens)" or (n > 1 and (" (x" .. n .. ")") or "")))
   for _ = 1, n do
      self:callLua("lua_missionStateSceneEventHandler", eventId, actor, parameter or 0)
   end
   self:runPending()
   return n
end

-- Answers and scene timeouts come from the scene actor, which has no name
local sceneActorName = ""

function Engine:deliverFromScene(eventId, parameter)
   -- Questions are passed on directly (no listener), timeouts per listener
   if eventId:sub(1, 2) == "Qs" then
      self:note("EVENT", string.format("%s (question %d)", eventId, parameter))
      self:callLua("lua_missionStateSceneEventHandler", eventId, sceneActorName, parameter)
      self:runPending()
   else
      self:deliver(sceneActorName, eventId, parameter)
   end
end

-- Lets time pass: scene timers tick once per update (60 per second)
function Engine:advance(updates)
   for _ = 1, updates do
      if not self.scene then
         return
      end
      local fired = {}
      for _, t in ipairs(self.timers) do
         t.ticks = t.ticks + 1
         if t.ticks > t.timeout and not t.done then
            t.done = true
            fired[#fired + 1] = t.id
         end
      end
      for _, id in ipairs(fired) do
         self:deliverFromScene("ScTO", id)
      end
   end
end

-- ------------------------------------------------------------------------
-- What a player does (the events the game would send)

function Engine:requireObject(name, what)
   local obj = self:getObject(name)
   if not obj then
      error(what .. ": no object '" .. tostring(name) .. "' in " .. (self.scene and self.scene.file or "any scene"), 3)
   end
   return obj
end

function Engine:leapfrog()
   if not self.scene then
      return nil
   end
   for _, obj in pairs(self.scene.objects) do
      if obj.isLeapfrog then
         return obj
      end
   end
   return nil
end

-- Fuel from a pad on landing, all at once (the game refuels over time)
function Engine:refuel(pad)
   local lf = self:leapfrog()
   if not lf or pad.props[2] == 0 then
      return
   end
   local space = 100 - self.resources.fuel
   if space <= 0 then
      return
   end
   if pad.props[4] == 2 then
      local supply = pad.props[3]
      local amount = (supply < 0) and space or math.min(space, supply)
      self.resources.fuel = self.resources.fuel + amount
      if supply >= 0 then pad.props[3] = supply - amount end
      self:note("FUEL", string.format("%.1f free from %s", amount, pad.name))
   elseif pad.props[4] == 1 then
      local price = pad.fuelPrice or 5
      local amount = math.min(space, math.floor(self.resources.credits / price))
      self.resources.fuel = self.resources.fuel + amount
      self.resources.credits = self.resources.credits - amount * price
      self:note("FUEL", string.format("%d bought at %s for %d credits", amount, pad.name, amount * price))
   end
end

function Engine:land(name)
   local obj = self:requireObject(name, "land")
   self.landedOn = obj
   if obj.behaviour == "landingPad" then
      obj.props[0], obj.props[1] = 1, 1
      if obj.props[2] ~= 0 then
         self:deliver(name, "LpLL", 0)
      end
      self:refuel(obj)
   elseif obj.behaviour == "launchSite" then
      if obj.props[2] ~= 0 then
         self:deliver(name, "LsLL", 0)
      end
   else
      error("land: '" .. name .. "' is neither a landing pad nor a launch site", 2)
   end
   self:updateQuestions()
end

-- Taking off: the booster fires, then the pad says so. With a question
-- shown the booster button is its Yes, so a test must answer it first.
function Engine:takeOff()
   self:boost()
   local obj = self.landedOn
   self.landedOn = nil
   if obj and obj.behaviour == "landingPad" then
      obj.props[0], obj.props[1] = 2, 0
      if obj.props[2] ~= 0 then
         self:deliver(obj.name, "LpTO", 0)
      end
   end
   if obj and obj.behaviour == "launchSite" and obj.launchTriggered then
      -- Left before the grabber took it: the launch is cancelled
      obj.launchTriggered = false
      self:note("LAUNCH", obj.name .. ": left, launch cancelled")
   end
   self:updateQuestions()
end

function Engine:boost()
   if self.question then
      error(string.format("the booster button is Yes while question %d is shown: answer it first", self.question), 4)
   end
end

function Engine:answer(id, yes)
   if self.question ~= id then
      error(string.format("answer: question %s is not open (open: %s, queued: %s)", tostring(id),
         tostring(self.question), table.concat(self.questionQueue or {}, ", ")), 3)
   end
   self.question = nil
   self:deliverFromScene(yes and "QsYs" or "QsNo", id)
   self:updateQuestions()
end

-- ------------------------------------------------------------------------
-- The c_* functions

function Engine:installApi(env)
   local e = self
   local api = {}

   api.c_startScene = function(file, sceneType)
      e:armScene(file, sceneType)
   end

   api.c_registerEventHandler = function(actor, eventId, _, _)
      if not e:getObject(actor) then
         -- The game shows an ERROR message in the dialog
         e:problem("c_registerEventHandler: no object '" .. tostring(actor) .. "' in " .. (e.scene and e.scene.file or "?"))
         return
      end
      local key = actor .. "/" .. eventId
      e.listeners[key] = (e.listeners[key] or 0) + 1
      e:note("LISTEN", key .. (e.listeners[key] > 1 and (" (x" .. e.listeners[key] .. ")") or ""))
   end

   api.c_registerPropertyTrigger = function(actor, eventId, propertyId, triggerType, lower, upper)
      local obj = e:getObject(actor)
      if not obj then
         e:problem("c_registerPropertyTrigger: no object '" .. tostring(actor) .. "'")
         return
      end
      obj.triggers[#obj.triggers + 1] = { eventId = eventId, propertyId = propertyId,
         type = triggerType, lower = lower, upper = upper }
   end

   api.c_clearAllTriggersAndEvents = function()
      e.listeners = {}
      if e.scene then
         for _, obj in pairs(e.scene.objects) do
            obj.triggers = {}
         end
      end
      e:note("CLEAR", "triggers and event handlers")
   end

   api.c_addMissionStateSceneObjects = function(file)
      local root = e:readXml(file)
      if not root or not e.scene then
         return
      end
      for _, c in ipairs(root.children) do
         if c.tag == "childObject" then
            local obj = e:objectFromChildNode(c)
            if obj then
               e.scene.objects[obj.name] = obj
            end
         end
      end
      e:note("OBJECTS", "added from " .. file)
   end

   api.c_addPositionedChildObject = function(file, name, x, y)
      if not e.scene then
         e:problem("c_addPositionedChildObject without a scene")
         return
      end
      local obj
      if file:match("^leapfrog") then
         obj = e:makeLeapfrog(name, file, x, y)
      else
         local root = e:readXml(file)
         if not root then return end
         obj = e:objectFromNode(name, root)
         obj.x, obj.y = x, y
      end
      e.scene.objects[name] = obj
      e:note("OBJECTS", string.format("%s from %s at (%s, %s)", name, file, tostring(x), tostring(y)))
   end

   api.c_addDialogMessage = function(message, sender, leftSide, preDelay, postDelay, affiliation)
      e.dialog[#e.dialog + 1] = { sender = sender, text = message, left = leftSide }
      e:note("DIALOG", sender .. ": " .. message)
   end

   api.c_setSenderAffiliation = function(sender, affiliation)
      local ok = ({ none = 1, leapfrog = 1, neutral = 1, enemy = 1, friendly = 1 })[affiliation]
      if not ok then
         e:problem("c_setSenderAffiliation: unknown affiliation '" .. tostring(affiliation) .. "'")
      end
      e.senders[sender] = affiliation
   end

   api.c_setSenderColor = function(sender, color)
      e.senderColors[sender] = color
   end

   api.c_hudLog = function(text, critical)
      e.hud[#e.hud + 1] = { text = text, critical = critical }
      e:note("HUD", text)
   end

   api.c_setPanningObject = function(name)
      if not e:getObject(name) then
         e:problem("c_setPanningObject: no object '" .. tostring(name) .. "'")
      end
   end

   api.c_setObjectProperty = function(name, id, value)
      local obj = e:getObject(name)
      if not obj then
         e:problem("c_setObjectProperty: no object '" .. tostring(name) .. "'")
         return
      end
      e:setProperty(obj, id, value)
   end

   api.c_getObjectProperty = function(name, id)
      local obj = e:getObject(name)
      if not obj then
         e:problem("c_getObjectProperty: no object '" .. tostring(name) .. "' (the game returns 0)")
         return 0.0
      end
      return e:getProperty(obj, id) + 0.0
   end

   api.c_objectExists = function(name)
      return e:getObject(name) ~= nil
   end

   local function mapItem(fn, name, actor)
      local obj = e:getObject(name)
      if not obj then
         e:problem(fn .. ": no object '" .. tostring(name) .. "'")
         return nil
      end
      if not obj.actors[actor] then
         e:problem(fn .. ": object '" .. name .. "' has no actor '" .. tostring(actor) .. "'")
         return nil
      end
      local key = name .. "/" .. actor
      e.map[key] = e.map[key] or { state = nil, shown = true, affiliation = nil }
      return e.map[key]
   end

   api.c_setObjectMapState = function(name, actor, state)
      local m = mapItem("c_setObjectMapState", name, actor)
      if m then m.state = state end
   end

   api.c_setObjectMapAffiliation = function(name, actor, affiliation)
      local m = mapItem("c_setObjectMapAffiliation", name, actor)
      if m then m.affiliation = affiliation end
   end

   api.c_setObjectMapShown = function(name, actor, shown)
      local m = mapItem("c_setObjectMapShown", name, actor)
      if m then m.shown = shown and true or false end
   end

   api.c_startSceneTimer = function(ticks)
      local id = e.timerCounter
      e.timerCounter = e.timerCounter + 1
      e.timers[#e.timers + 1] = { id = id, ticks = 0, timeout = ticks }
      -- Like the game, every timer adds a listener for ScTO on the scene
      local key = sceneActorName .. "/ScTO"
      e.listeners[key] = (e.listeners[key] or 0) + 1
      return id + 0.0
   end

   api.c_exitSceneSaveGameStatus = function(scene, mission, state, sceneType, exitScene, exitType, exitParam)
      local s = e.status
      s.scene, s.mission, s.state, s.sceneType = scene, mission, state, sceneType
      s.exitScene, s.exitType, s.exitParam = exitScene, exitType, exitParam
      e:saveGameStatus()
   end

   api.c_saveGameStatus = function(scene, mission, state, sceneType)
      -- readGameStatus, setSceneMissionState (in memory), then the file is
      -- written as it was: the scene is saved when it has started
      e:readGameStatus()
      local s = e.status
      s.scene, s.mission, s.state, s.sceneType = scene, mission, state, sceneType
      s.exitScene, s.exitType, s.exitParam = "", -1, -1
   end

   api.c_askQuestion = function(id, flightText, sender)
      if type(id) ~= "number" or id < 0 then
         e:problem("c_askQuestion: the question id must be 0 or more")
         return
      end
      -- Already open or queued: nothing happens
      if e.question == id then
         e:note("QUESTION", string.format("%d asked again (already open)", id))
         return
      end
      for _, q in ipairs(e.questionQueue) do
         if q == id then
            e:note("QUESTION", string.format("%d asked again (already queued)", id))
            return
         end
      end
      table.insert(e.questionQueue, id)
      if type(flightText) == "string" and flightText ~= "" then
         e.questionFlight[id] = { text = flightText, sender = sender or "", given = false }
      else
         e.questionFlight[id] = nil
      end
      e:note("QUESTION", "asked " .. id .. " (queued)")
   end

   api.c_leapfrogStandsOn = function()
      if e:leapfrogStands() then
         return e.landedOn.name
      end
      return nil
   end

   api.c_cancelQuestion = function(id)
      if id == nil or id < 0 then
         e.question = nil
         e.questionQueue = {}
         return
      end
      if e.question == id then
         e.question = nil
      end
      for i = #e.questionQueue, 1, -1 do
         if e.questionQueue[i] == id then
            table.remove(e.questionQueue, i)
         end
      end
   end

   self.dialog = self.dialog or {}
   self.hud = self.hud or {}
   self.senders = self.senders or {}
   self.senderColors = self.senderColors or {}

   -- Every function the game registers must be here
   for _, name in ipairs(self.registered) do
      if api[name] then
         env[name] = api[name]
      elseif name:sub(1, 2) == "c_" then
         self:problem("the tester has no mock for " .. name .. " (new in luainterface.cpp?)")
      end
   end
end

return Engine
