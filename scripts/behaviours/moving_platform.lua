local platform = {}

function platform:OnCreate()
    local transform =
        scene.GetComponent(
            self.ID,
            "transform"
        )

    self.startX = transform.x
    self.distance = 150

    self.speed = 100
    self.direction = 1
end

function platform:OnUpdate(delta)
    while true do
        local transform =
            scene.GetComponent(
                self.ID,
                "transform"
            )

        if transform.x >= self.startX + self.distance then
            self.direction = -1
        elseif transform.x <= self.startX then
            self.direction = 1
        end

        scene.SetComponent(
            self.ID,
            "velocity",
            self.speed * self.direction,
            0
        )

        delta = coroutine.yield()
    end
end

return platform
