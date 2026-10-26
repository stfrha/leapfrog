-- The test harness: test registration, the play-through steps and checks a
-- test uses (the "t" object), and the pass/fail report.
--
-- A test:
--
--   test("Bruno's job, accepted", function(t)
--      t:newGame()
--      local S = t:mission(1).State
--      t:expectDialog("Do you want the job?", "Bruno")
--      t:answer(t:mission(1).Question.acceptJob, true)
--      t:expectState(S.accepted)
--   end)
--
-- Every step checks that the engine saw no problem (a Lua error, an unknown
-- object, ...). A failed check stops the test and is reported with the
-- file and line of the check and what happened just before.

local Engine = require("testbench.engine")

local harness = {
   tests = {},
   dataDir = nil,
   srcDir = nil,
   verbose = false,
}

local Failure = {}
Failure.__index = Failure

-- ------------------------------------------------------------------------
-- Registration

function harness.test(name, fn)
   harness.tests[#harness.tests + 1] = { name = name, fn = fn, file = harness.currentFile }
end

-- A test of how the game should behave but doesn't yet. It is reported as
-- KNOWN (not a failure) while it fails, with the reason; once it passes
-- it is reported as FIXED and counts as a failure until known() is
-- changed to test(), so the suite keeps up with the game.
function harness.known(name, reason, fn)
   harness.tests[#harness.tests + 1] = { name = name, fn = fn, file = harness.currentFile, known = reason }
end

-- ------------------------------------------------------------------------
-- The test object

local T = {}
T.__index = T

local function newT()
   local t = setmetatable({}, T)
   t.engine = Engine.new(harness.dataDir, harness.srcDir)
   t.checks = 0
   t.dialogMark = 0
   t.hudMark = 0
   t.allowed = {}
   return t
end

-- Where in the test file a check was made
local function callerLine(level)
   local info = debug.getinfo(level + 1, "Sl")
   if info then
      return (info.short_src or "?") .. ":" .. (info.currentline or 0)
   end
   return "?"
end

function T:fail(message, level)
   error(setmetatable({ message = message, where = callerLine((level or 1) + 1) }, Failure), 0)
end

-- Names for a state number, from the mission modules' State tables
function T:stateName(value)
   local env = self.engine.env
   if env and env.package then
      for name, mod in pairs(env.package.loaded) do
         if type(mod) == "table" and type(mod.State) == "table" then
            for k, v in pairs(mod.State) do
               if v == value then
                  return k
               end
            end
         end
      end
   end
   return nil
end

local function show(t, value)
   local n = (type(value) == "number") and t:stateName(value) or nil
   return tostring(value) .. (n and (" (" .. n .. ")") or "")
end

-- Fails on engine problems the test didn't say it expects
function T:checkProblems(level)
   local e = self.engine
   if #e.problems == 0 then
      return
   end
   local left = {}
   for _, p in ipairs(e.problems) do
      local ok = false
      for _, pattern in ipairs(self.allowed) do
         if p:find(pattern, 1, true) then ok = true end
      end
      if not ok then left[#left + 1] = p end
   end
   e.problems = {}
   if #left > 0 then
      self:fail("engine problem: " .. table.concat(left, "\n       "), (level or 1) + 1)
   end
end

-- Runs a step, turning a Lua error inside it into a failure at the caller
local function step(t, fn, ...)
   local ok, err = pcall(fn, ...)
   if not ok then
      if getmetatable(err) == Failure then error(err, 0) end
      t:fail(tostring(err), 3)
   end
   t:checkProblems(3)
end

-- ------------------------------------------------------------------------
-- Steps: what the player (or the game) does

-- A new game on a fresh install: no save file, the game starts
function T:newGame()
   step(self, function() self.engine.file = nil; self.engine:boot() end)
end

-- Quit the game and start it again: a new Lua state, the save file stays
function T:restartGame()
   step(self, function() self.engine:boot() end)
end

-- The menu's "new game" (same Lua state as before)
function T:menuNewGame()
   step(self, function() self.engine:menuNewGame() end)
end

-- The menu's "continue" (same Lua state as before)
function T:menuContinue()
   step(self, function() self.engine:continueGame() end)
end

-- Touch down on a landing pad or launch site (both feet, within limits)
function T:land(name)
   step(self, function() self.engine:land(name) end)
end

-- Take off from where the Leapfrog stands (the booster fires)
function T:takeOff()
   step(self, function() self.engine:takeOff() end)
end

-- Fire the main booster without taking off (withdraws an open question)
function T:boost()
   step(self, function() self.engine:boost() end)
end

-- Answer the open question
function T:answer(id, yes)
   step(self, function() self.engine:answer(id, yes) end)
end

-- Let time pass (scene timers run, 60 updates per second)
function T:wait(seconds)
   step(self, function() self.engine:advance(math.floor(seconds * 60 + 0.5)) end)
end

-- Leave the scene, as the game does: landing scene to deep space (0, 1),
-- deep space to orbit (1, 3), orbit to a landing site (2, index)
function T:exitScene(sceneType, parameter)
   step(self, function() self.engine:exitScene(sceneType, parameter) end)
end

-- Land on a launch site and launch: the site must have been triggered
-- (property 1) by the script, then the scene exits to deep space
function T:launch(site)
   step(self, function() self.engine:land(site) end)
   local obj = self.engine:getObject(site)
   if not obj or not obj.launchTriggered then
      self:fail("launch: the script did not trigger " .. site .. " (property 1)", 2)
   end
   step(self, function() self.engine:exitScene(0, 1) end)
end

-- A game event from an object (e.g. "BrBR" a breakable broke, "ExEX" an
-- explosive exploded, "PuPU" a pickup was picked up), sent to listeners
function T:event(object, eventId, parameter)
   step(self, function()
      self.engine:requireObject(object, "event")
      self.engine:deliver(object, eventId, parameter or 0)
   end)
end

-- Sets a global of the scripts (e.g. currentMission = 0 for free play)
function T:setGlobal(name, value)
   self.engine.env[name] = value
end

-- Sets the scene up again, as the scripts do after a state change
function T:setupScene()
   step(self, function()
      self.engine:callLua("lua_setupMissionStateScene")
      self.engine:runPending()
   end)
end

-- The main menu's "test landing" entry (the old test mission)
function T:menuTestLanding()
   step(self, function() self.engine:menuTestLanding() end)
end

-- Change a property from the game's side (e.g. damage the legs)
function T:setProperty(object, id, value)
   step(self, function()
      local e = self.engine
      local obj = e:requireObject(object, "setProperty")
      if obj.isLeapfrog and id == 10 then
         -- The game damages joints directly; this sets all of them
         for i = 1, #e.status.legs do e.status.legs[i] = value end
      else
         e:setProperty(obj, id, value)
      end
   end)
end

-- Sets the lowest leg joint health (one joint), as a crash would
function T:damageLegs(health)
   step(self, function() self.engine.status.legs[1] = health end)
end

-- Let a check pass with this engine problem (a substring of it)
function T:allowProblem(pattern)
   self.allowed[#self.allowed + 1] = pattern
end

-- ------------------------------------------------------------------------
-- Looking at the game

function T:mission(n)
   local env = self.engine.env
   local mod = env and env.package.loaded["mission" .. n]
   if type(mod) ~= "table" then
      self:fail("mission " .. n .. "'s module is not loaded", 2)
   end
   return mod
end

function T:module(name)
   local env = self.engine.env
   local mod = env and env.package.loaded[name]
   if type(mod) ~= "table" then
      self:fail("module " .. name .. " is not loaded", 2)
   end
   return mod
end

function T:global(name)
   return self.engine.env and self.engine.env[name]
end

function T:property(object, id)
   local obj = self.engine:getObject(object)
   if not obj then
      self:fail("no object '" .. object .. "'", 2)
   end
   return self.engine:getProperty(obj, id)
end

-- Dialog messages since the last expectDialog / clearDialog
function T:newDialog()
   local d = self.engine.dialog
   local out = {}
   for i = self.dialogMark + 1, #d do out[#out + 1] = d[i] end
   return out
end

function T:clearDialog()
   self.dialogMark = #self.engine.dialog
   self.hudMark = #self.engine.hud
end

-- ------------------------------------------------------------------------
-- Checks

local function pass(t)
   t.checks = t.checks + 1
end

function T:expect(condition, message)
   if not condition then self:fail(message or "expectation failed", 2) end
   pass(self)
end

function T:expectEqual(actual, expected, what)
   if actual ~= expected then
      self:fail(string.format("%s: expected %s, got %s", what or "value",
         show(self, expected), show(self, actual)), 2)
   end
   pass(self)
end

function T:expectScene(file)
   local s = self.engine.scene
   if not s or s.file ~= file then
      self:fail(string.format("expected scene %s, in %s", file, s and s.file or "no scene"), 2)
   end
   pass(self)
end

function T:expectMission(n)
   local m = self:global("currentMission")
   if m ~= n then
      self:fail(string.format("expected mission %s, got %s", tostring(n), tostring(m)), 2)
   end
   pass(self)
end

function T:expectState(value)
   local s = self:global("currentState")
   if s ~= value then
      self:fail(string.format("expected state %s, got %s", show(self, value), show(self, s)), 2)
   end
   pass(self)
end

-- A dialog message containing text (and from sender, if given) since the
-- last dialog check. Moves past it, so the next check looks further on.
function T:expectDialog(text, sender)
   local d = self.engine.dialog
   for i = self.dialogMark + 1, #d do
      local m = d[i]
      if m.text:find(text, 1, true) and (sender == nil or m.sender == sender) then
         self.dialogMark = i
         pass(self)
         return m
      end
   end
   local lines = {}
   for i = self.dialogMark + 1, #d do lines[#lines + 1] = d[i].sender .. ": " .. d[i].text end
   self:fail(string.format("expected dialog \"%s\"%s; new dialog:\n         %s", text,
      sender and (" from " .. sender) or "",
      #lines > 0 and table.concat(lines, "\n         ") or "(none)"), 2)
end

-- No new dialog at all (text nil), or none containing text
function T:expectNoDialog(text)
   local d = self.engine.dialog
   for i = self.dialogMark + 1, #d do
      if text == nil or d[i].text:find(text, 1, true) then
         self:fail("unexpected dialog: " .. d[i].sender .. ": " .. d[i].text, 2)
      end
   end
   pass(self)
end

-- How many new dialog messages contain text
function T:countDialog(text)
   local n = 0
   for _, m in ipairs(self:newDialog()) do
      if m.text:find(text, 1, true) then n = n + 1 end
   end
   return n
end

function T:expectHud(text)
   local h = self.engine.hud
   for i = self.hudMark + 1, #h do
      if h[i].text:find(text, 1, true) then
         self.hudMark = i
         pass(self)
         return
      end
   end
   self:fail("expected HUD log \"" .. text .. "\"", 2)
end

function T:expectQuestion(id)
   local q = self.engine.question
   if q ~= id then
      self:fail(string.format("expected question %s to be open, open: %s", tostring(id), tostring(q)), 2)
   end
   pass(self)
end

function T:expectNoQuestion()
   if self.engine.question ~= nil then
      self:fail("expected no open question, open: " .. tostring(self.engine.question), 2)
   end
   pass(self)
end

function T:expectProperty(object, id, expected, tolerance)
   local v = self:property(object, id)
   if math.abs(v - expected) > (tolerance or 1e-6) then
      self:fail(string.format("%s property %d: expected %s, got %s", object, id, tostring(expected), tostring(v)), 2)
   end
   pass(self)
end

local mapStates = { hollow = 0, filled = 1, flashingSlow = 2, flashingFast = 3 }

-- state: "hollow", "filled", "flashingSlow", "flashingFast" (or a number)
function T:expectMap(object, actor, state)
   local want = mapStates[state] or state
   local m = self.engine.map[object .. "/" .. actor]
   local got = m and m.state
   if got ~= want then
      self:fail(string.format("map %s/%s: expected state %s, got %s", object, actor,
         tostring(state), tostring(got)), 2)
   end
   pass(self)
end

function T:expectMapShown(object, actor, shown)
   local m = self.engine.map[object .. "/" .. actor]
   local got = (m == nil) and true or m.shown
   if got ~= shown then
      self:fail(string.format("map %s/%s: expected shown = %s, got %s", object, actor,
         tostring(shown), tostring(got)), 2)
   end
   pass(self)
end

-- Someone listens to the event, count times (default exactly once)
function T:expectListening(object, eventId, count)
   local n = self.engine:listenerCount(object, eventId)
   if n ~= (count or 1) then
      self:fail(string.format("%s %s: expected %d listener(s), got %d", object, eventId, count or 1, n), 2)
   end
   pass(self)
end

function T:expectSaved(field, expected)
   local f = self.engine.file
   local v = f and f[field]
   if v ~= expected then
      self:fail(string.format("save file %s: expected %s, got %s", field, show(self, expected), show(self, v)), 2)
   end
   pass(self)
end

-- ------------------------------------------------------------------------
-- Running and reporting

local function recentLog(engine, n)
   local out = {}
   local first = math.max(1, #engine.log - n + 1)
   for i = first, #engine.log do
      local l = engine.log[i]
      out[#out + 1] = string.format("%-8s %s", l.kind, l.text)
   end
   return out
end

function harness.run(filter)
   local passed, failed, known, checks = 0, 0, 0, 0
   local failures = {}

   for _, test in ipairs(harness.tests) do
      if filter == nil or test.name:find(filter, 1, true) then
         local t = newT()
         local ok, err = pcall(test.fn, t)

         if ok then
            -- Anything left at the end counts too
            ok, err = pcall(function() t:checkProblems(1) end)
         end

         checks = checks + t.checks
         if test.known and not ok then
            known = known + 1
            local message = (getmetatable(err) == Failure) and err.message or tostring(err)
            print(string.format("[KNOWN] %s", test.name))
            print("       " .. test.known)
            print("       (fails with: " .. message:gsub("\n.*", "") .. ")")
         elseif test.known and ok then
            failed = failed + 1
            print(string.format("[FIXED] %s", test.name))
            print("       passes now: change known() to test() in " .. (test.file or "?"))
            failures[#failures + 1] = test.name
         elseif ok then
            passed = passed + 1
            print(string.format("[PASS] %s (%d checks)", test.name, t.checks))
         else
            failed = failed + 1
            local message, where
            if getmetatable(err) == Failure then
               message, where = err.message, err.where
            else
               message, where = tostring(err), test.file or "?"
            end
            print(string.format("[FAIL] %s", test.name))
            print("       " .. message)
            print("       at " .. where)
            print("       last events:")
            for _, line in ipairs(recentLog(t.engine, 12)) do
               print("         " .. line)
            end
            failures[#failures + 1] = test.name
         end

         if harness.verbose then
            print("       log:")
            for _, line in ipairs(recentLog(t.engine, #t.engine.log)) do
               print("         " .. line)
            end
         end
      end
   end

   print(string.rep("-", 60))
   print(string.format("%d tests: %d passed, %d failed, %d known issue%s (%d checks)",
      passed + failed + known, passed, failed, known, known == 1 and "" or "s", checks))
   if failed > 0 then
      print("FAILED: " .. table.concat(failures, ", "))
   end

   return failed == 0, passed + failed + known
end

return harness
