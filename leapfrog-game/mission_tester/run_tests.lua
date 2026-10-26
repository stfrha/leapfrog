-- Mission tester: runs mission progression tests against the game's real
-- Lua scripts with a mock engine. See README.md.
--
--   mission_tester [--data <dir>] [--src <dir>] [--filter <text>] [--verbose]
--                  [test files...]
--
-- Without test files it runs the ones listed in tests/test_list.lua.
-- Exit code 0 when all tests pass, 1 when any fails, 2 on a setup error.

local root = TESTER_ROOT
if root == nil then
   -- Run with a plain Lua interpreter: the folder of this script
   root = (arg and arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
end

package.path = root .. "/?.lua;" .. package.path

local harness = require("testbench.harness")

local dataDir = root .. "/../data"
local srcDir = root .. "/../src"
local filter = nil
local files = {}

local i = 1
while arg and i <= #arg do
   local a = arg[i]
   if a == "--data" then
      i = i + 1; dataDir = arg[i]
   elseif a == "--src" then
      i = i + 1; srcDir = arg[i]
   elseif a == "--filter" then
      i = i + 1; filter = arg[i]
   elseif a == "--verbose" or a == "-v" then
      harness.verbose = true
   elseif a == "--help" or a == "-h" then
      print("mission_tester [--data <dir>] [--src <dir>] [--filter <text>] [--verbose] [test files...]")
      return 0
   else
      files[#files + 1] = a
   end
   i = i + 1
end

harness.dataDir = dataDir
harness.srcDir = srcDir

if #files == 0 then
   local list = dofile(root .. "/tests/test_list.lua")
   for _, f in ipairs(list) do
      files[#files + 1] = root .. "/tests/" .. f
   end
end

print("Leapfrog mission tester")
print("  game data: " .. dataDir)
print("  game src:  " .. srcDir)
print(string.rep("-", 60))

-- Load the test files: each calls test(name, function(t) ... end)
_G.test = harness.test
_G.known = harness.known
for _, f in ipairs(files) do
   harness.currentFile = f
   local chunk, err = loadfile(f)
   if not chunk then
      print("[ERROR] cannot load " .. f .. ": " .. tostring(err))
      return 2
   end
   local ok, e = pcall(chunk)
   if not ok then
      print("[ERROR] " .. f .. ": " .. tostring(e))
      return 2
   end
end

local ok, count = harness.run(filter)

if count == 0 then
   print("[ERROR] no tests ran" .. (filter and (" (filter \"" .. filter .. "\")") or ""))
   return 2
end

return ok and 0 or 1
