-- all the objects created inside the ECS and placable
-- in a level
local objects = {}

-- table of object definitions, used for creating and updating objects
objects.definitions = {
    spike = {
        width = 25,
        height = 60,
        color = { 220, 40, 40 }
    },

    platform = {
        width = 150,
        height = 25,
        color = { 180, 140, 50 },

        speed = 100,
        distance = 150
    }
}


function objects.Create(objectType, x, y, ghost)
    local definition =
        objects.definitions[objectType]

    if definition == nil then
        error(
            "Unknown object type: "
            .. tostring(objectType)
        )
    end

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
        definition.width,
        definition.height
    )

    local alpha = ghost and 120 or 255

    scene.SetComponent(
        entity,
        "renderable",
        definition.color[1],
        definition.color[2],
        definition.color[3],
        alpha
    )

    -- Ghost has no gameplay components.
    if ghost then
        return entity
    end

    if objectType == "spike" then
        scene.SetComponent(
            entity,
            "hazard"
        )
    elseif objectType == "platform" then
        scene.SetComponent(
            entity,
            "velocity",
            definition.speed,
            0
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
    end

    return entity
end

function objects.UpdateGhost(entity, objectType)
    local definition =
        objects.definitions[objectType]

    if definition == nil then
        return
    end

    scene.SetComponent(
        entity,
        "collider",
        definition.width,
        definition.height
    )

    scene.SetComponent(
        entity,
        "renderable",
        definition.color[1],
        definition.color[2],
        definition.color[3],
        120
    )
end

return objects
