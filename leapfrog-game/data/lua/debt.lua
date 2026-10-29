-- Debt: Bruno's credit. When something has to be paid and the credits
-- (Leapfrog property 8) don't cover it, Bruno pays the rest and the
-- credits go below zero: a negative balance is what the pilot owes him
-- (the HUD shows it in red). Credits earned fill the hole first, and
-- anything that checks for enough credits (fuel at the pads, repairs)
-- stays closed while it lasts. Bruno says when he lends, and when he has
-- been paid back.
--
-- mission_script.lua calls debt.setupScene at every scene setup, last
-- (after the mission, so the Leapfrog exists), and debt.eventHandler for
-- every event, first.

local debt = {}

local lender = "Bruno"
local leapfrog = "leapfrog1"
local creditsProperty = 8

-- Bruno has lent and not been paid back (also set from a negative balance
-- found at a scene setup, e.g. after loading a game)
local owing = false

-- Lines to say at the next scene setup (charges made while a scene ends:
-- what is said then would be dropped with the scene)
local pendingLines = {}

local function say(text, later)
   if later then
      pendingLines[#pendingLines + 1] = text
   else
      c_addDialogMessage(text, lender, true, 60, 0)
   end
end

-- Whole credits; nil if there is no Leapfrog in the scene
local function credits()
   if not c_objectExists(leapfrog) then
      return nil
   end
   return math.floor(c_getObjectProperty(leapfrog, creditsProperty))
end

local function paidBack()
   owing = false
   say("Got your money. We're square.")
end

-- Watches the credits (a property trigger, "OpTr") while something is owed
local function watch()
   if owing and c_objectExists(leapfrog) then
      c_registerPropertyTrigger(leapfrog, 0, creditsProperty, "insideRange", 0, 1000000000)
      listen(leapfrog, "OpTr")
   end
end

-- What is owed now (0 if nothing)
debt.owed = function()
   local c = credits()
   if c == nil or c >= 0 then
      return 0
   end
   return -c
end

-- Charges amount for what (e.g. "the recovery crew"). What the credits
-- don't cover, Bruno lends; he says so now, or with later = true at the
-- next scene setup. Returns what the credits paid and what Bruno lent.
debt.charge = function(amount, what, later)
   local c = credits()
   if c == nil or amount <= 0 then
      return 0, 0
   end

   local covered = math.max(0, math.min(c, amount))
   local lent = amount - covered
   c_setObjectProperty(leapfrog, creditsProperty, c - amount)

   if lent > 0 then
      owing = true
      say("I've covered " .. what .. " for you. You owe me " .. (amount - c) .. " credits.", later)
      if not later then
         watch()
      end
   end

   return covered, lent
end

debt.isOwing = function()
   return owing
end


-- ------------------------------------------------------------------------
-- Scene setup, last

debt.setupScene = function()
   for _, text in ipairs(pendingLines) do
      say(text)
   end
   pendingLines = {}

   local c = credits()
   if c == nil then
      return
   end

   if c < 0 then
      owing = true
      watch()
   elseif owing then
      -- Paid back while the scene changed
      paidBack()
   end
end


-- ------------------------------------------------------------------------
-- Events, first

debt.eventHandler = function(eventId, actorName, parameter1)
   if eventId == "OpTr" and actorName == leapfrog and parameter1 == creditsProperty then
      local c = credits()
      if owing and c ~= nil and c >= 0 then
         paidBack()
      end
   end
end

return debt
