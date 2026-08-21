-- create a simple rectangle terrain
-- TODO: have it use texture instead of flat colour
local terrain = {}

function terrain.Create(x, y, width, height, color)
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
        width,
        height
    )

    scene.SetComponent(
        entity,
        "renderable",
        color[1],
        color[2],
        color[3]
    )

    scene.SetComponent(
        entity,
        "solid"
    )

    return entity
end

return terrain
