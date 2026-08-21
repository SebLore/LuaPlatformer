local controller = {}

function controller:OnCreate()
    self.moveSpeed = 300
    self.jumpSpeed = 550
end

function controller:OnUpdate(delta)
    while true do
        local velocity =
            scene.GetComponent(
                self.ID,
                "velocity"
            )

        local player =
            scene.GetComponent(
                self.ID,
                "player"
            )

        velocity.x = 0

        if input.KeyDown("A")
            or input.KeyDown("LEFT") then
            velocity.x = -self.moveSpeed
        end

        if input.KeyDown("D")
            or input.KeyDown("RIGHT") then
            velocity.x = self.moveSpeed
        end

        if (
                input.KeyPressed("SPACE")
                or input.KeyPressed("W")
                or input.KeyPressed("UP")
            ) and player.grounded then
            velocity.y = -self.jumpSpeed
        end

        scene.SetComponent(
            self.ID,
            "velocity",
            velocity.x,
            velocity.y
        )

        delta = coroutine.yield()
    end
end

return controller
