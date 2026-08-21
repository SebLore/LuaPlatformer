local terrain = require "scripts.level.terrain"
terrain.Create()


local gameController = scene.CreateEntity()

scene.SetComponent(
    gameController,
    "behaviour",
    "scripts/controllers/game_controller.lua"
)
