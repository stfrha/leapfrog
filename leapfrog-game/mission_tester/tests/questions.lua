-- Yes/No questions (c_askQuestion): asked any time, answered only with the
-- Leapfrog standing on a landing pad or launch site; a launch waits for them

local Q_TEST = 77    -- not a question any script handles

local function atOrb(t)
   t:newGame()
   t:land("siloLandingPad")
   t:answer(t:mission(1).Question.acceptJob, true)
   t:takeOff()
   t:land("oreLanding")
   t:expectState(t:mission(1).State.atOrb)
   t:takeOff()
end

test("questions: asking one that is open or queued again adds nothing", function(t)
   t:newGame()
   t:askQuestion(Q_TEST)
   t:askQuestion(Q_TEST)
   t:land("siloLandingPad")
   -- Bruno's offer first (asked first), then this one, once
   t:answer(t:mission(1).Question.acceptJob, false)
   t:expectQuestion(Q_TEST)
   t:askQuestion(Q_TEST)
   t:answer(Q_TEST, true)
   t:expectNoQuestionPending()
end)

test("questions: one asked on a pad opens at once", function(t)
   atOrb(t)
   t:land("landingPad2")
   t:askQuestion(Q_TEST)
   t:expectQuestion(Q_TEST)
end)

test("questions: a launch waits for a pending question, then goes", function(t)
   atOrb(t)
   -- Asked in flight, on the way to the launch site
   t:askQuestion(Q_TEST)
   t:expectQuestionQueued(Q_TEST)

   -- Landing triggers the launch (mission 1), and the question opens
   t:land("launchSite1")
   t:expectQuestion(Q_TEST)
   t:expect(t.engine:getObject("launchSite1").launchTriggered, "the script triggers the launch")

   t:answer(Q_TEST, true)
   t:completeLaunch("launchSite1")
   t:expectScene("deep_space_scene.xml")
end)

test("questions: a launch on hold can be aborted (property 3)", function(t)
   atOrb(t)
   t:askQuestion(Q_TEST)
   t:land("launchSite1")
   t:expectQuestion(Q_TEST)

   -- As a handler answering "no" would
   t:setProperty("launchSite1", 3, 1)
   t:answer(Q_TEST, false)
   t:expect(not t.engine:getObject("launchSite1").launchTriggered, "the launch is aborted")
   t:expectProperty("launchSite1", 3, 0)
   t:expectScene("home_scene.xml")
end)

test("questions: c_leapfrogStandsOn names the pad or launch site, nil in flight", function(t)
   atOrb(t)
   local where = function() return t.engine.env.c_leapfrogStandsOn() end
   t:expectEqual(where(), nil, "in flight")
   t:land("landingPad2")
   t:expectEqual(where(), "landingPad2", "on a pad")
   t:takeOff()
   t:land("launchSite1")
   t:expectEqual(where(), "launchSite1", "on the launch site")
end)

test("questions: the in-flight line is said only if still in flight when the dialog catches up", function(t)
   atOrb(t)
   t:clearDialog()
   -- Asked in flight: the line comes
   t:askQuestion(Q_TEST, "A test?", "Bruno", "Land and we'll talk.")
   t:expectDialog("Land and we'll talk.", "Bruno")
   t:land("landingPad2")
   t:answer(Q_TEST, true)

   -- Asked on a pad: no line
   t:clearDialog()
   t:askQuestion(Q_TEST + 1, "Another test?", "Bruno", "Land and we'll talk.")
   t:expectNoDialog("Land and we'll talk.")
   t:expectQuestion(Q_TEST + 1)
end)

test("questions: the text of one asked while another is open waits for its turn", function(t)
   atOrb(t)
   t:land("landingPad2")
   t:clearDialog()
   t:askQuestion(Q_TEST, "First?", "Bruno")
   t:askQuestion(Q_TEST + 1, "Second?", "Silo")
   t:expectDialog("First?", "Bruno")
   t:expectNoDialog("Second?")
   t:expectQuestion(Q_TEST)
   t:answer(Q_TEST, true)
   t:expectDialog("Second?", "Silo")
   t:expectQuestion(Q_TEST + 1)
end)

test("questions: one asked in flight: its text at once, the in-flight line after, not again on landing", function(t)
   atOrb(t)
   t:clearDialog()
   t:askQuestion(Q_TEST, "Once?", "Bruno", "Land and we'll talk.")
   t:expectDialog("Once?", "Bruno")
   t:expectDialog("Land and we'll talk.", "Bruno")
   t:land("landingPad2")
   t:expectQuestion(Q_TEST)
   t:expectNoDialog("Once?")
end)
