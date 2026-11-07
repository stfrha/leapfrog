-- documentation/lua_guide.md stays true to the code: blocks marked as coming
-- from a file must still be in that file, and the c_ functions it names must
-- exist. Blocks are compared without each line's indent. They look like this:
--
--   <!-- verbatim: data/lua/mission_stash.lua -->
--   ```lua
--   ...
--   ```

local harness = require("testbench.harness")

local function readFile(path)
   local f = io.open(path, "rb")
   if f == nil then
      return nil
   end
   local text = f:read("a"):gsub("\r\n", "\n")
   f:close()
   return text
end

local function guidePath()
   return harness.dataDir .. "/../documentation/lua_guide.md"
end

-- The text without the indent and trailing spaces of its lines
local function unindented(text)
   return (("\n" .. text):gsub("\n[ \t]+", "\n"):gsub("[ \t]+\n", "\n"))
end

-- Returns a list of { file, code, line } for the guide's marked blocks
local function markedBlocks(text)
   local blocks = {}
   local file, code, startLine
   local n = 0
   for line in (text .. "\n"):gmatch("(.-)\n") do
      n = n + 1
      local marked = line:match("<!%-%- verbatim: (.-) %-%->")
      if marked ~= nil then
         file = marked
      elseif file ~= nil and code == nil and line:match("^%s*```") then
         code = {}
         startLine = n
      elseif code ~= nil then
         if line:match("^%s*```%s*$") then
            blocks[#blocks + 1] = { file = file, code = table.concat(code, "\n"), line = startLine }
            file, code = nil, nil
         else
            code[#code + 1] = line
         end
      end
   end
   return blocks
end

test("guide: the code it quotes is still in the files", function(t)
   local text = readFile(guidePath())
   t:expect(text ~= nil, "documentation/lua_guide.md can't be read")

   local blocks = markedBlocks(text)
   t:expect(#blocks > 0, "the guide has no marked code blocks")

   for _, block in ipairs(blocks) do
      local source = readFile(harness.dataDir .. "/../" .. block.file)
      t:expect(source ~= nil, "can't read " .. block.file)
      t:expect(unindented(source):find(unindented(block.code), 1, true) ~= nil,
         "lua_guide.md line " .. block.line .. ": this code is no longer in " .. block.file)
   end
end)

test("guide: the c_ functions it names exist", function(t)
   local text = readFile(guidePath())
   local source = readFile(harness.srcDir .. "/luainterface.cpp")
   t:expect(text ~= nil and source ~= nil, "the guide or luainterface.cpp can't be read")

   for name in text:gmatch("[^%w_](c_%l[%w_]*)") do
      t:expect(source:find('lua_register(m_L, "' .. name .. '"', 1, true) ~= nil,
         "lua_guide.md names " .. name .. ", which luainterface.cpp doesn't register")
   end
end)

-- The text of the guide's section 7
local function reference(text)
   local start = text:find("\n## 7%. Reference")
   return start and text:sub(start) or ""
end

test("guide: the reference lists every registered c_ function", function(t)
   local text = reference(readFile(guidePath()))
   local source = readFile(harness.srcDir .. "/luainterface.cpp")
   t:expect(text ~= "", "the guide has no section 7")

   for line in source:gmatch("[^\n]+") do
      local name = line:match('^%s*lua_register%(m_L, "(c_[%w_]+)"')
      if name ~= nil then
         t:expect(text:find("`" .. name .. "(", 1, true) ~= nil,
            name .. " isn't in section 7 of lua_guide.md")
      end
   end
end)

-- The event codes declared in the game's headers, without commented-out
-- ones. The headers are the ones proj/leapfrog.vcxproj lists: Lua can't
-- list a folder on every platform.
local function sourceEvents()
   local codes = {}
   local project = readFile(harness.dataDir .. "/../proj/leapfrog.vcxproj") or ""
   for file in project:gmatch('ClInclude Include="%.%.\\src\\([^"]+%.h)"') do
      local text = readFile(harness.srcDir .. "/" .. file) or ""
      for line in text:gmatch("[^\n]+") do
         if not line:match("^%s*//") then
            local a, b, c, d = line:match("EVENT = eventID%('(.)', '(.)', '(.)', '(.)'%)")
            if a ~= nil then
               codes[a .. b .. c .. d] = true
            end
         end
      end
   end
   return codes
end

test("guide: the reference and the source agree on the events", function(t)
   local text = reference(readFile(guidePath()))
   local codes = sourceEvents()
   t:expect(next(codes) ~= nil, "no events found in the source")

   for code in pairs(codes) do
      t:expect(text:find("`" .. code .. "`", 1, true) ~= nil,
         "event " .. code .. " isn't in section 7 of lua_guide.md")
   end

   local inTables = text:sub(text:find("### Events for scripts", 1, true) or 1,
      (text:find("### Object properties", 1, true) or #text))
   for code in inTables:gmatch("`(%a%a%a%a)`") do
      t:expect(codes[code] or code == "QsYs" or code == "QsNo",
         "lua_guide.md lists event " .. code .. ", which the source doesn't declare")
   end
end)

test("guide: the test steps it names exist", function(t)
   local text = readFile(guidePath())
   local harnessText = readFile(package.searchpath("testbench.harness", package.path))
   t:expect(harnessText ~= nil, "testbench/harness.lua can't be read")

   for name in text:gmatch("[^%w_]t:(%l[%w_]*)%(") do
      t:expect(harnessText:find("function T:" .. name .. "(", 1, true) ~= nil,
         "lua_guide.md names t:" .. name .. ", which harness.lua doesn't have")
   end
end)
