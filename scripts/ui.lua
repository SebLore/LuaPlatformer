-- for now handles creating text as entities, later buttons and more

local ui = {}

function ui.CreateText(x, y, text, fontSize, r, g, b)
    local entity = scene.CreateEntity()

    scene.SetComponent(
        entity,
        "transform",
        x,
        y
    )

    scene.SetComponent(
        entity,
        "ui_text",
        text,
        fontSize,
        r,
        g,
        b
    )

    return entity
end

function ui.SetText(entity, text, fontSize, r, g, b)
    scene.SetComponent(
        entity,
        "ui_text",
        text,
        fontSize,
        r,
        g,
        b
    )
end

return ui
