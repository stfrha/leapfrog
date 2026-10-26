-- A small XML reader, enough for the game's scene and object files:
-- elements with attributes, nested; text, comments and the declaration
-- are skipped. Returns a tree of nodes { tag, attr = {..}, children = {..} }.

local xml = {}

local function parseAttributes(text)
   local attr = {}
   for name, value in text:gmatch('([%w_:%.%-]+)%s*=%s*"([^"]*)"') do
      attr[name] = value
   end
   return attr
end

-- Parses an XML string. Returns the root element, or nil and an error.
function xml.parse(text)
   -- Drop a UTF-8 byte order mark, comments and the declaration
   text = text:gsub("^\239\187\191", "")
   text = text:gsub("<!%-%-.-%-%->", "")
   text = text:gsub("<%?.-%?>", "")

   local root = { tag = "#document", attr = {}, children = {} }
   local stack = { root }

   for slash, tag, attrText, selfClose in text:gmatch("<(/?)([%w_:%.%-]+)(.-)(/?)>") do
      local top = stack[#stack]
      if slash == "/" then
         if top.tag ~= tag then
            return nil, "mismatched </" .. tag .. "> (open: <" .. top.tag .. ">)"
         end
         stack[#stack] = nil
      else
         local node = { tag = tag, attr = parseAttributes(attrText), children = {}, parent = top }
         top.children[#top.children + 1] = node
         if selfClose ~= "/" then
            stack[#stack + 1] = node
         end
      end
   end

   if #stack ~= 1 then
      return nil, "unclosed <" .. stack[#stack].tag .. ">"
   end

   return root.children[1]
end

function xml.readFile(path)
   local f = io.open(path, "rb")
   if not f then
      return nil, "cannot open " .. path
   end
   local text = f:read("a")
   f:close()
   return xml.parse(text)
end

-- The first direct child with this tag
function xml.child(node, tag)
   for _, c in ipairs(node.children) do
      if c.tag == tag then
         return c
      end
   end
   return nil
end

return xml
