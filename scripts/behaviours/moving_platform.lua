local platform = {}

local objects =
    require "scripts.level.objects"


function platform:OnCreate()
    local transform =
        scene.GetComponent(
            self.ID,
            "transform"
        )

    self.startX = transform.x
    self.direction = 1
end

function platform:OnUpdate(delta)
    while true do
        local transform =
            scene.GetComponent(
                self.ID,
                "transform"
            )

        local definition =
            objects.definitions.platform

        if transform.x >=
            self.startX + definition.distance then
            self.direction = -1
        elseif transform.x <= self.startX then
            self.direction = 1
        end

        scene.SetComponent(
            self.ID,
            "velocity",
            definition.speed * self.direction,
            0
        )

        delta = coroutine.yield()
    end
end

return platform
