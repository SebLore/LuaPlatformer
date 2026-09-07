-- create a simple rectangle terrain
-- TODO: have it use texture instead of flat colour
local terrain = {}

function terrain.Create(x, y, width, height)
    local entity = scene.CreateEntity()

    scene.SetComponent(entity, "transform",x, y)

    scene.SetComponent(
        entity,
        "collider",
        width,
        height
    )

    scene.SetComponent(
        entity,
        "renderable",
        80,
        80,
        80
    )

    scene.SetComponent(
        entity,
        "solid"
    )

    return entity
end

return terrain
