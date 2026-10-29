# Mission tester

Runs mission progression tests against the game's real Lua scripts
(`data/scene_navigator.lua`, `data/mission_script.lua`, `data/lua/...`) with
a mock engine in place of the C++ game. A test plays the game in steps
("new game", "answer yes", "land on landingPad2", "launch") and checks what
the scripts did: the mission state, the dialog, the question asked, the map,
properties, what was saved.

It tests the story logic and the scripts' bookkeeping, not physics or
timing: a test says that the Leapfrog lands, and the mock engine sends the
events the game would. Play-testing stays.

## Running

**Windows (Visual Studio).** The `mission_tester` project is in
`leapfrog.sln`. Building it also runs all tests (post-build step); the
output window shows the results, and a failing test fails the build of this
project. *Debug → Start* (with `mission_tester` as startup project) runs
them in a console. From a command prompt, after a build:

    leapfrog-game\mission_tester\run_tests.bat [arguments]

**macOS (and Linux).**

    leapfrog-game/mission_tester/build.sh              # build and run all tests
    leapfrog-game/mission_tester/build.sh norun        # build only
    leapfrog-game/mission_tester/build.sh -- --filter "mission 1" --verbose

Both build `main.c`, a small Lua host, against the Lua sources next to the
repo (`../../../lua` from this folder, the same checkout the game uses);
nothing else is needed.

**Arguments**

    --filter <text>   only tests whose name contains the text
    --verbose, -v     print everything that happened in each test
    --data <dir>      the game's data folder (default ../data)
    --src <dir>       the game's src folder (default ../src)
    --root <dir>      this folder (found automatically)
    <files>           run these test files instead of tests/test_list.lua

Exit code: 0 all passed, 1 a test failed (or a known issue is fixed), 2
set-up error.

## Output

    [PASS] mission 1: the route moves on pad by pad (6 checks)
    [FAIL] mission 1: flying to the Orb without the job, Bruno asks there
           expected state 12 (atOrb), got 11 (accepted)
           at ./tests/mission1_home.lua:97
           last events:
             EVENT    LpLL oreLanding 0
             DIALOG   Bruno: You came anyway. So, do you want the job?
             ...
    [KNOWN] saving: continuing at home keeps the mission's progress
           the game saves only at scene exits ...
    ------------------------------------------------------------
    31 tests: 29 passed, 0 failed, 2 known issues (118 checks)

States are shown with their names from the missions' `State` tables.

## Writing tests

Test files are in `tests/` and listed in `tests/test_list.lua`. A test:

```lua
test("mission 1: accepting puts fuel in the pads", function(t)
   t:newGame()
   local m = t:mission(1)               -- the mission module: m.State, m.Question
   t:land("siloLandingPad")             -- Bruno's question, asked in flight, opens
   t:answer(m.Question.acceptJob, true)
   t:expectState(m.State.accepted)
   t:expectProperty("landingPad2", 4, 2)
   t:expectMap("landingPad2", "siloHome", "flashingSlow")
end)
```

`known(name, reason, fn)` is a test of how the game should behave but
doesn't yet: reported as KNOWN with the reason while it fails, and as FIXED
(a failure) once it passes, so it gets turned into a `test`.

**Steps**

| Step | What it does |
|---|---|
| `t:newGame()` | fresh install: no save file, the game starts |
| `t:restartGame()` | quit and start again: new Lua state, the save file stays |
| `t:menuNewGame()`, `t:menuContinue()` | the main menu's entries (same Lua state) |
| `t:menuTestStart(i)`, `t:testStarts()` | the menu's test row: start test start i of `lua/test_starts.lua` (fails if refused); the labels in menu order |
| `t:menuTestLanding()` | test start 1: the old test mission in the landing scene, from orbit |
| `t:land(name)` | touch down on a landing pad (LpLL, fuel) or launch site (LsLL) |
| `t:takeOff()` | the booster fires, LpTO (an error while a question is open: the booster button is its Yes) |
| `t:boost()` | the booster fires without taking off (same rule) |
| `t:answer(id, yes)` | answers the open question (must be that id) |
| `t:askQuestion(id)` | a script asks a question (for tests of the question rules) |
| `t:wait(seconds)` | scene timers run (60 updates a second) |
| `t:launch(site)` | land on the site; the script must trigger it; to deep space |
| `t:completeLaunch(site)` | the launch of the site the Leapfrog stands on goes (triggered, not aborted, no question pending) |
| `t:exitScene(type, param)` | a scene exit (landing 0, deep space 1, orbit 2) |
| `t:reentry(outcome [, site, distance])` | the orbit scene ends: `"onSite"`/`"offSite"` (closest site index, distance off in planet units; a marker is 100 wide), `"bounced"`, `"burnedUp"`, `"missed"`; `c_lastReentry` returns it, then the scene exits |
| `t:event(object, id [, param])` | a game event, e.g. `"BrBR"`, `"ExEX"`, `"PuPU"` |
| `t:setProperty(object, id, v)` | change a property from the game's side |
| `t:damageLegs(h)` | one leg joint at health h (property 10 reads the lowest) |
| `t:setGlobal(name, v)`, `t:setupScene()` | e.g. free play: `currentMission = 0`, set up again |
| `t:allowProblem(text)` | don't fail on an engine problem containing text |

Questions work as in the game: `c_askQuestion` queues them in the order
asked, and the first opens when the Leapfrog stands on a landing pad or
launch site (`t:land`); taking off before answering is not possible, and a
launch site's launch waits while one is pending (property 3 aborts it).

**Checks**

`expectScene`, `expectMission`, `expectState`, `expectDialog(text [, sender])`
(searches new dialog, then moves past the match), `expectNoDialog([text])`,
`clearDialog`, `expectHud`, `expectQuestion(id)` (open), `expectNoQuestion` (none open),
`expectQuestionQueued(id)`, `expectNoQuestionPending` (none open or queued),
`expectProperty(object, id, value [, tolerance])`,
`expectMap(object, actor, "hollow"|"filled"|"flashingSlow"|"flashingFast")`,
`expectMapShown(object, actor, bool)`, `expectListening(object, event [, count])`,
`expectSaved(field, value)` (fields: scene, mission, state, sceneType,
exitScene, exitType, exitParam, fuel, credits, ...), `expectEqual`, `expect`.

Reading: `t:property(object, id)`, `t:global(name)`, `t:module(name)`,
`t:newDialog()`, `t:countDialog(text)`.

## What the mock engine checks by itself

Every step fails the test if the engine saw a problem:

- a Lua error in a script (the game logs it and goes on);
- an object that isn't in the scene (`c_registerEventHandler` shows an ERROR
  in the game; property and map calls are silently ignored there);
- a map actor (body) the object doesn't have;
- an unknown affiliation;
- a `c_*` function the game registers that the tester has no mock for, and
  modules the game preloads: both are read from `src/luainterface.cpp`, so a
  new function or script shows up here.

Objects come from the scene XML files (`childObject`s, also from `file=`),
with their shapes as map actors and their behaviour (landing pad, launch
site) for properties and events. Events are delivered once per registered
listener, as in the game, so a double registration shows as a double event.

## Files

    main.c               the Lua host (Windows and macOS)
    mission_tester.vcxproj
    build.sh             build and run on macOS / Linux
    run_tests.bat        run on Windows after a build
    run_tests.lua        entry: arguments, loads the tests, prints the summary
    testbench/engine.lua the mock engine (MainActor, GameStatus, LuaInterface, ...)
    testbench/harness.lua steps, checks, reporting
    testbench/xml.lua    reads the scene files
    tests/               the tests
