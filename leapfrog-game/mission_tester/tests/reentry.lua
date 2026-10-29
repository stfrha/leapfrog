-- Re-entry outcomes (scene_navigator.lua, lua_reentryOutcome in
-- mission_script.lua, mining.reentryOutcome, lua/debt.lua): on a site, off
-- it (the recovery crew's fee; Bruno's credit when the credits don't
-- cover it), bounced back to deep space, burned up

-- Mission 1 up to the orbit scene: the job, the Orb, the launch, deep
-- space's lower right corner
local function toOrbit(t)
   t:newGame()
   t:land("siloLandingPad")
   t:answer(t:mission(1).Question.acceptJob, true)
   t:takeOff()
   t:land("oreLanding")
   t:takeOff()
   t:launch("launchSite1")
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
   t:clearDialog()
end

-- Where the Leapfrog came in
local function arrival(t)
   local lf = t.engine:leapfrog()
   return lf and lf.file, lf and lf.x, lf and lf.y
end

test("re-entry: on a site: its landing scene, coming in from above, nothing to pay", function(t)
   toOrbit(t)
   t:setProperty("leapfrog1", 8, 30)
   t:reentry("onSite", 1)                 -- Alpha City: landing_scene for now
   t:expectScene("landing_scene.xml")
   t:expectSaved("scene", "landing_scene.xml")
   t:expectProperty("leapfrog1", 8, 30)
   t:expectNoDialog("Recovery")
   local file, x, y = arrival(t)
   t:expectEqual(file, "leapfrog_reentry.xml", "Leapfrog file")
   t:expectEqual(y, 50, "from above")
end)

test("re-entry: the gold mine site is home for now, the silo's pad", function(t)
   toOrbit(t)
   t:reentry("onSite", 0)
   t:expectScene("home_scene.xml")
   local file, x, y = arrival(t)
   t:expectEqual(file, "leapfrog_reentry.xml", "Leapfrog file")
   t:expectEqual(x, 2736, "above the silo")
end)

test("re-entry: recovered: standing on the site's pad, also after a continue", function(t)
   toOrbit(t)
   t:reentry("offSite", 1, 100)
   t:expectScene("landing_scene.xml")
   local file, x, y = arrival(t)
   t:expectEqual(file, "leapfrog_landing.xml", "Leapfrog file")
   t:expectEqual(x, 415, "on landing_pad1")
   t:expectEqual(y, 657, "on landing_pad1")
   t:expectSaved("exitParam", 101)

   t:restartGame()
   t:expectScene("landing_scene.xml")
   file, x, y = arrival(t)
   t:expectEqual(file, "leapfrog_landing.xml", "Leapfrog file after a continue")
   t:expectEqual(x, 415, "still on the pad")
end)

test("re-entry: recovered at home: on the silo's pad", function(t)
   toOrbit(t)
   t:reentry("offSite", 0, 50)
   t:expectScene("home_scene.xml")
   local file, x, y = arrival(t)
   t:expectEqual(file, "leapfrog_landing.xml", "Leapfrog file")
   t:expectEqual(x, 2736, "on the silo's pad")
   t:expectEqual(y, 500, "on the silo's pad")
end)

test("re-entry: off the site: the recovery crew's fee, by the distance", function(t)
   toOrbit(t)
   local fee = t:module("mining").recoveryFee(100)     -- one marker width off
   t:setProperty("leapfrog1", 8, fee + 10)
   t:reentry("offSite", 1, 100)
   t:expectScene("landing_scene.xml")
   t:expectDialog("set you down on the pad. That's " .. fee .. " credits.", "Recovery")
   t:expectHud("Recovery: " .. fee .. " credits")
   t:expectProperty("leapfrog1", 8, 10)
   t:expectSaved("credits", 10)                       -- the fee is in the save
   t:expectNoDialog("You owe me")
end)

test("re-entry: a farther miss costs more", function(t)
   local fee = function(d) return t:module("mining").recoveryFee(d) end
   toOrbit(t)
   t:expect(fee(300) > fee(100) and fee(100) > fee(10), "the fee grows with the distance")
end)

test("re-entry: off the site without the credits: Bruno lends, the balance goes negative", function(t)
   toOrbit(t)
   local fee = t:module("mining").recoveryFee(100)
   t:setProperty("leapfrog1", 8, 10)
   t:reentry("offSite", 0, 100)
   t:expectProperty("leapfrog1", 8, 10 - fee)
   t:expectSaved("credits", 10 - fee)
   t:expectDialog("That's " .. fee .. " credits.", "Recovery")
   t:expectDialog("I've covered the recovery crew for you. You owe me " .. (fee - 10) .. " credits.", "Bruno")
   t:expect(t:module("debt").isOwing(), "Bruno is owed")
end)

test("re-entry: paying Bruno back: he says so, once", function(t)
   toOrbit(t)
   t:setProperty("leapfrog1", 8, 0)
   t:reentry("offSite", 0, 100)
   t:clearDialog()

   -- Earning something, still in debt: nothing said
   t:setProperty("leapfrog1", 8, -5)
   t:expectNoDialog("square")

   -- Back to zero or more: paid
   t:setProperty("leapfrog1", 8, 15)
   t:expectDialog("We're square.", "Bruno")
   t:expect(not t:module("debt").isOwing(), "nothing owed")
   t:setProperty("leapfrog1", 8, 40)
   t:expectNoDialog("square")              -- once
end)

test("re-entry: a debt found at a scene setup (e.g. after loading) is watched", function(t)
   toOrbit(t)
   t:setProperty("leapfrog1", 8, -20)
   t:reentry("onSite", 0)
   t:expect(t:module("debt").isOwing(), "the negative balance is a debt")
   t:clearDialog()
   t:setProperty("leapfrog1", 8, 0)
   t:expectDialog("We're square.", "Bruno")
end)

test("re-entry: bounced: deep space again, the strippers after a breather", function(t)
   toOrbit(t)
   t:reentry("bounced")
   t:expectScene("deep_space_scene.xml")
   t:expectSaved("scene", "deep_space_scene.xml")
   t:expectHud("Bounced off the atmosphere")
   t:expect(not t.engine:getObject("swarm"), "no swarm yet")
   t:wait(11)
   t:expect(t.engine:getObject("swarm") ~= nil, "the swarm has started")

   -- The way to orbit is open again
   t:exitScene(1, 3)
   t:expectScene("orbit_scene.xml")
end)

test("re-entry: burned up: back to the latest save (the orbit, as entered)", function(t)
   toOrbit(t)
   t:setProperty("leapfrog1", 8, 99)       -- not saved: lost with the Leapfrog
   t:reentry("burnedUp")
   t:expectScene("orbit_scene.xml")
   t:expectHud("Burned up on re-entry")
   t:expect(t:property("leapfrog1", 8) ~= 99, "the save's credits, not those since")
end)
