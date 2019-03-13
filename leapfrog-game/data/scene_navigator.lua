-- currentScene and gameProgress should be read from file to begin with
-- perhaps they are read from files and then sent as inputs to this script

currentScene = "New Game"
gameProgress = 0


-- entryType is: toGround, toDeepSpace, toOrbit, toHyperspace
-- entryParameter is additional information related to an enty
function determineNextScene(entryType, entryParameter)
   local nextScene = ""
   local sceneType = ""
   local sceneState = ""
   
   if currentScene == "New Game" then
      nextScene = "landing_scene.xml"
      sceneType = "landing"
      sceneState = "groundState"
   
   elseif currentScene == "landing_scene.xml" then
      if entryType == "toDeepSpace" then
         nextScene = "deep_space_scene.xml"
         sceneType = "deepSpace"
         sceneState = "fromGround"
      else
         nextScene = "Current scene is " .. currentScene .. ", could not find currentState"
         sceneType = ""
         sceneState = "entryType is: " .. entryType .. " and it was not found"
      end

   elseif currentScene == "orbit_scene.xml" then
      if entryType == "toGround" then
         if entryParameter == "alphaCity" then
            nextScene = "landing_scene.xml"
            sceneType = "landing"
            sceneState = "landingState"
         elseif entryParameter == "goldMine" then
            nextScene = "mine_scene.xml"
            sceneType = "landing"
            sceneState = "landingState"
         else
            nextScene = "Current scene is " .. currentScene .. ", could not find currentState"
            sceneType = ""
            sceneState = "entryType is: " .. entryType .. " and it was not found"
         end 
      end

   elseif currentScene == "deep_space_scene.xml" then
      if entryType == "toOrbit" then
         nextScene = "orbit_scene.xml"
         sceneType = "orbit"
         sceneState = "fromDeepSpace"
      elseif entryType == "toHyperspace" then
         nextScene = "hyper_scene.xml"
         sceneType = "hyperSpace"
         sceneState = "fromDeepSpace"
      else
         nextScene = "Current scene is " .. currentScene .. ", could not find currentState"
         sceneType = ""
         sceneState = "entryType is: " .. entryType .. " and it was not found"

      end
   else
      nextScene = "Current scene is " .. currentScene .. ", could not find currentState"
      sceneType = ""
      sceneState = "entryType is: " .. entryType .. " and it was not found"
   end
   
   currentScene = nextScene

   return nextScene, sceneState, sceneType
   
end

