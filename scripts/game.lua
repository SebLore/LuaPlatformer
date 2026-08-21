-- main entry point for the game

local level = require "scripts.level.level"
level.Create()

-- this registers as an entity with a behaviour in the C++ environment and 
-- will handle the game loop
-- TODO: add  controllers for level editor and menu(s)
local gameController = scene.CreateEntity()
scene.SetComponent(
    gameController,
    "behaviour",
    "scripts/controllers/game_controller.lua"
)
