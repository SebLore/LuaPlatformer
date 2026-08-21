dofile("scripts/common.lua")

local playerColors = {
    colors.blue,
    colors.red,
    colors.green,
    colors.yellow
}

local player = {}

function player.Create(x, y, number)
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
        0,
        0
    )

    scene.SetComponent(
        entity,
        "collider",
        40,
        60
    )

    -- set player color based on number
    local color = playerColors[number] or { 255, 255, 255 }

    scene.SetComponent(
        entity,
        "renderable",
        color[1],
        color[2],
        color[3]
    )

    scene.SetComponent(
        entity,
        "player"
    )

    return entity
end

return player