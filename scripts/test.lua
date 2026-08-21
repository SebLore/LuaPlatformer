print("Lua started")

local entity = scene.CreateEntity()

print("Created entity:", entity)

scene.SetComponent(
    entity,
    "transform",
    300,
    200
)

scene.SetComponent(entity, "velocity", 10, 0)

print("Transform added")