-- TODO: use terrain.lua
local terrain = {}

terrain.start = {
    x = 200,
    y = 500
}

function terrain.Create()
    -- floor
    local floor = scene.CreateEntity()

    scene.SetComponent(floor, "transform", 100, 600)
    scene.SetComponent(floor, "collider", 1000, 60)
    scene.SetComponent(floor, "renderable", 80, 80, 80)
    scene.SetComponent(floor, "solid")


    -- platform
    local platform = scene.CreateEntity()

    scene.SetComponent(platform, "transform", 550, 450)
    scene.SetComponent(platform, "collider", 250, 30)
    scene.SetComponent(platform, "renderable", 100, 100, 100)
    scene.SetComponent(platform, "solid")


    -- start marker
    local start = scene.CreateEntity()

    scene.SetComponent(
        start,
        "transform",
        terrain.start.x,
        terrain.start.y + 150
    )

    scene.SetComponent(
        start,
        "collider",
        40,
        5
    )

    scene.SetComponent(
        start,
        "renderable",
        255,
        128,
        0
    )


    -- goal
    local goal = scene.CreateEntity()

    scene.SetComponent(goal, "transform", 1000, 500)
    scene.SetComponent(goal, "collider", 50, 100)
    scene.SetComponent(goal, "renderable", 40, 200, 80)
    scene.SetComponent(goal, "goal")
end

return terrain
