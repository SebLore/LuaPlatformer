local controller = {}

local playerModule = require "scripts.player"
local objects = require "scripts.objects"
local ui = require "scripts.ui"


function controller:OnCreate()
    self.playerCount = 2
    self.maxRounds = 3

    self.round = 1
    self.currentPlayer = 1

    self.scores = {}

    for i = 1, self.playerCount do
        self.scores[i] = 0
    end

    self.activePlayerEntity = nil
    self.uiEntity = nil

    self.spawnX = 200
    self.spawnY = 500

    self.phase = "PLACEMENT"
    self.selectedItem = "spike"

    self:CreateUI()

    print("Game started")
    print("Round:", self.round)

    print(
        "Placement: Player",
        self.currentPlayer,
        "- press 1/2 and click"
    )
end

function controller:CreateUI()
    self.uiEntity =
        ui.CreateText(
            20,
            20,
            "",
            28,
            0,
            0,
            0
        )
end

function controller:UpdateUI()
    local text = ""

    if self.phase == "PLACEMENT" then
        local itemName = self.selectedItem

        if self.selectedItem == "spike" then
            itemName = "Spike"
        elseif self.selectedItem == "platform" then
            itemName = "Moving Platform"
        end

        text =
            "Round "
            .. self.round
            .. " - Player "
            .. self.currentPlayer
            .. " Placing "
            .. itemName
    elseif self.phase == "PLAYING" then
        text =
            "Round "
            .. self.round
            .. " - Player "
            .. self.currentPlayer
            .. " Playing"
    elseif self.phase == "GAME_OVER" then
        text = "Game Over"
    end

    ui.SetText(
        self.uiEntity,
        text,
        28,
        0,
        0,
        0
    )
end

function controller:SpawnPlayer()
    self.activePlayerEntity =
        playerModule.Create(
            self.spawnX,
            self.spawnY,
            self.currentPlayer
        )

    print(
        "Player",
        self.currentPlayer,
        "started"
    )
end

function controller:FinishAttempt(success)
    if success then
        self.scores[self.currentPlayer] =
            self.scores[self.currentPlayer] + 1

        print(
            "Player",
            self.currentPlayer,
            "scored!"
        )
    else
        print(
            "Player",
            self.currentPlayer,
            "failed"
        )
    end

    if self.activePlayerEntity ~= nil then
        if scene.IsEntity(self.activePlayerEntity) then
            scene.RemoveEntity(
                self.activePlayerEntity
            )
        end

        self.activePlayerEntity = nil
    end

    self.currentPlayer =
        self.currentPlayer + 1

    if self.currentPlayer > self.playerCount then
        self.currentPlayer = 1
        self.round = self.round + 1

        if self.round > self.maxRounds then
            self.phase = "GAME_OVER"
            self:GameOver()
        else
            self.phase = "PLACEMENT"

            print(
                "Round",
                self.round,
                "placement"
            )
        end
    end
end

function controller:GameOver()
    print("GAME OVER")

    for i = 1, self.playerCount do
        print(
            "Player",
            i,
            "score:",
            self.scores[i]
        )
    end
end

function controller:UpdatePlacement()
    if input.KeyPressed("1") then
        self.selectedItem = "spike"
        print("Selected spike")
    end

    if input.KeyPressed("2") then
        self.selectedItem = "platform"
        print("Selected moving platform")
    end

    if not input.MousePressed() then
        return
    end

    local x, y =
        input.GetMousePosition()

    if self.selectedItem == "spike" then
        objects.CreateSpike(x, y)
    elseif self.selectedItem == "platform" then
        objects.CreateMovingPlatform(x, y)
    end

    print(
        "Player",
        self.currentPlayer,
        "placed",
        self.selectedItem
    )

    self.currentPlayer =
        self.currentPlayer + 1

    if self.currentPlayer > self.playerCount then
        self.currentPlayer = 1
        self.phase = "PLAYING"

        print("Placement finished")
        print("Player 1 starts")
    else
        print(
            "Placement: Player",
            self.currentPlayer
        )
    end
end

function controller:OnUpdate(delta)
    while true do
        self:UpdateUI()

        if self.phase == "PLACEMENT" then
            self:UpdatePlacement()
        elseif self.phase == "PLAYING" then
            if self.activePlayerEntity == nil then
                self:SpawnPlayer()
            elseif not scene.IsEntity(
                    self.activePlayerEntity
                ) then
                self.activePlayerEntity = nil
            else
                local state =
                    scene.GetComponent(
                        self.activePlayerEntity,
                        "player"
                    )

                if state ~= nil then
                    if state.dead then
                        self:FinishAttempt(false)
                    elseif state.reachedGoal then
                        self:FinishAttempt(true)
                    else
                        local transform =
                            scene.GetComponent(
                                self.activePlayerEntity,
                                "transform"
                            )

                        if transform ~= nil
                            and transform.y > 800 then
                            self:FinishAttempt(false)
                        end
                    end
                end
            end
        end

        delta = coroutine.yield()
    end
end

return controller
