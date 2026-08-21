require "scripts.level.objects"

local terrain = {}

function terrain.Create()

    -- entity
    local e = scene.CreateEntity()

    scene.SetComponent(
        e,
        "transform",
        100,
        600
    )

    -- ground
    scene.SetComponent(
        e,
        "collider",
        1000,
        60
    )

    scene.SetComponent(
        e,
        "renderable",
        80,
        80,
        80
    )

    scene.SetComponent(
        e,
        "solid"
    )




    -- Raised platform
    local platform = scene.CreateEntity()

    scene.SetComponent(
        platform,
        "transform",
        550,
        450
    )

    scene.SetComponent(
        platform,
        "collider",
        250,
        30
    )

    scene.SetComponent(
        platform,
        "renderable",
        100,
        100,
        100
    )

    scene.SetComponent(
        platform,
        "solid"
    )

    -- Temporary static spike hazard
    local spike = scene.CreateEntity()

    scene.SetComponent(
        spike,
        "transform",
        500,
        570
    )

    scene.SetComponent(
        spike,
        "collider",
        50,
        80
    )

    scene.SetComponent(
        spike,
        "renderable",
        220,
        40,
        40
    )

    scene.SetComponent(
        spike,
        "hazard"
    )

    -- Add goal
    local goal = scene.CreateEntity()

    scene.SetComponent(
        goal,
        "transform",
        1000,
        500
    )

    scene.SetComponent(
        goal,
        "collider",
        50,
        100
    )

    scene.SetComponent(
        goal,
        "renderable",
        40,
        200,
        80
    )

    scene.SetComponent(
        goal,
        "goal"
    )
end

return terrain