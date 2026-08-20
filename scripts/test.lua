print("Lua started")

local entity = scene.CreateEntity()

print("Created entity:", entity)

scene.SetComponent(
    entity,
    "transform",
    300,
    200
)

print("Transform added")