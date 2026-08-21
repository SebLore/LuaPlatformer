local behaviour = {}

function behaviour:OnCreate()
    self.time = 0

    print("OnCreate entity:", self.ID)
end

function behaviour:OnUpdate(delta)
    while true do
        self.time = self.time + delta

        if self.time >= 1.0 then
            print(
                "Behaviour running:",
                self.ID
            )

            self.time = 0
        end

        delta = coroutine.yield()
    end
end

return behaviour
