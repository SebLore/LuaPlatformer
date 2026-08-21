local terrain =
    dofile("scripts/terrain.lua")

terrain.Create()


local gameController =
    scene.CreateEntity()

scene.SetComponent(
    gameController,
    "behaviour",
    "scripts/behaviours/game_controller.lua"
)
