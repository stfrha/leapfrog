-- This script is the master for all missions. It checks the currentMission variable and 
-- decodes this into a specific LUA-script for that mission. It then executes the functions 
-- of that file. 

local mission1 = require('mission1')


-- Global variables
currentState = 1


--function tprint (t, s)
--    for k, v in pairs(t) do
--        local kfmt = '["' .. tostring(k) ..'"]'
--        if type(k) ~= 'string' then
--            kfmt = '[' .. k .. ']'
--        end
--        local vfmt = '"'.. tostring(v) ..'"'
--        if type(v) == 'table' then
--            tprint(v, (s or '')..kfmt)
--        else
--            if type(v) ~= 'string' then
--                vfmt = tostring(v)
--            end
--            io.write(type(t)..(s or '')..kfmt..' = '..vfmt)
--        end
--    end
--end

-- This function is to be called when a new scene has been started
-- It does all setup for that scene in this mission state
function lua_setupMissionStateScene()

   -- io.output("mylog.txt")
   -- print("It really works this time!")
   --io.write("It really works this time!")
   -- print("Hello hello hello hello hello hello hello hello hello hello ")
   --io.write("Hello hello hello hello hello hello hello hello hello hello ")
   -- tprint(package.preload)
   -- Test if LF shall be positioned according to the nextLfxxxx
   -- variables
   if lfDefined == false then
      if useInitialLfPos == false then
         -- Note that if the file loaded is not the same configuration as 
         -- the nextLfMode, the LF will reconfigure as an animation
         c_addPositionedChildObject(nextLfFile, "leapfrog1", nextLfPosX, nextLfPosY)
         c_setObjectProperty("leapfrog1", 1, nextLfMode)
         lfDefined = true      
      end
   end

   -- Clear triggers and event handler
   c_clearAllTriggersAndEvents()
   if currentMission == 1 then
      mission1.setupMissionStateScene()
   end

end

function lua_missionStateSceneEventHandler(eventId, actorName, parameter1)
   if currentMission == 1 then
      mission1.missionStateSceneEventHandler(eventId, actorName, parameter1)
   end
end









