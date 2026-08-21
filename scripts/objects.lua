local objects = {}

function objects.CreateSpike(x, y)
    local entity = scene.CreateEntity()

    scene.SetComponent(
        entity,
        "transform",
        x,
        y
    )

    scene.SetComponent(
        entity,
        "collider",
        50,
        30
    )

    scene.SetComponent(
        entity,
        "renderable",
        220,
        40,
        40
    )

    scene.SetComponent(
        entity,
        "hazard"
    )

    return entity
end

function objects.CreateMovingPlatform(x, y)
    local entity = scene.CreateEntity()

    scene.SetComponent(
        entity,
        "transform",
        x,
        y
    )

    scene.SetComponent(
        entity,
        "velocity",
        100,
        0
    )

    scene.SetComponent(
        entity,
        "collider",
        150,
        25
    )

    scene.SetComponent(
        entity,
        "renderable",
        180,
        140,
        50
    )

    scene.SetComponent(
        entity,
        "solid"
    )

    scene.SetComponent(
        entity,
        "behaviour",
        "scripts/behaviours/moving_platform.lua"
    )

    return entity
end

return objects
