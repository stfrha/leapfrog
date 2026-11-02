-- documentation/lua_guide.md stays true to the code: blocks marked as coming
-- from a file must still be in that file, and the c_ functions it names must
-- exist. Blocks are compared without each line's indent. They look like this:
--
--   <!-- verbatim: data/lua/mission_2.lua -->
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
