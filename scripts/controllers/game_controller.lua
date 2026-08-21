-- main game controller that handles the game loop, player turns, and scoring
local playerModule = require "scripts.player"
local ui = require "scripts.ui"
local objects = require "scripts.level.objects"
local levelIO = require "scripts.level.level_io"
local level   = require "scripts.level.level"

local controller = {}

function controller:OnCreate()
    -- game specific data
    self.playerCount = 2
    self.maxRounds = 3
    self.round = 1
    self.currentPlayer = 1
    self.activePlayerEntity = nil

    -- game state
    self.phase = "PLACEMENT"    -- always start on placement phase
    self.selectedItem = "spike" -- default to spike
    self.placedObjects = {}
    self.gameOverText = ""

    -- scores for each player, initialize to 0
    self.scores = {}
    for i = 1, self.playerCount do
        self.scores[i] = 0
    end

    -- ui 
    self.statusUI = nil
    self.scoreUI = nil

    -- for handling ghosts during placement
    self.ghostEntity = nil
    self.ghostOffset = 0
    self.ghostDirection = 1

    -- level specific data TODO: move to level
   self.spawnX = level.start.x
   self.spawnY = level.start.y

    self:CreateUI()

    -- debug printing
    print("Game started")
    print("Round:", self.round)
end

function controller:CreateUI()
    self.statusUI =
        ui.CreateText(
            20, 20, -- xy
            "",     -- text
            28,     -- font size
            0,      -- color r
            0,      -- color g
            0       -- color b
        )

    self.scoreUI =
        ui.CreateText(
            20,
            55,
            "",
            24,
            0,
            0,
            0
        )
end

function controller:UpdateUI()
    -- ui text
    local statusText = ""

    -- placement phase
    if self.phase == "PLACEMENT" then
        local itemName = self.selectedItem

        if self.selectedItem == "spike" then
            itemName = "Spike"
        elseif self.selectedItem == "platform" then
            itemName = "Moving Platform"
        end

        statusText =
            "Round "
            .. self.round
            .. "/"
            .. self.maxRounds
            .. " - Player "
            .. self.currentPlayer
            .. " Placing "
            .. itemName
            .. "   [1/2 Select] [S Save] [L Load]"

    elseif self.phase == "PLAYING" then
        statusText =
            "Round "
            .. self.round
            .. "/"
            .. self.maxRounds
            .. " - Player "
            .. self.currentPlayer
            .. " Playing"
    elseif self.phase == "GAME_OVER" then
        statusText = self.gameOverText
    end

    local scoreText =
        "Player 1: "
        .. self.scores[1]
        .. "    Player 2: "
        .. self.scores[2]

    ui.SetText(
        self.statusUI,
        statusText,
        28,
        0,
        0,
        0
    )

    ui.SetText(
        self.scoreUI,
        scoreText,
        24,
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

function controller:DestroyGhost()
    if self.ghostEntity == nil then
        return
    end

    if scene.IsEntity(self.ghostEntity) then
        scene.RemoveEntity(
            self.ghostEntity
        )
    end

    self.ghostEntity = nil
end

function controller:CreateGhost()
    self:DestroyGhost()

    self.ghostOffset = 0
    self.ghostDirection = 1

    local x, y =
        input.GetMousePosition()

    self.ghostEntity =
        objects.Create(
            self.selectedItem,
            x,
            y,
            true
        )
end

function controller:UpdateGhost(delta)
    if self.ghostEntity == nil then
        self:CreateGhost()
    end

    local x, y =
        input.GetMousePosition()

    if self.selectedItem == "platform" then
        local definition =
            objects.definitions.platform

        self.ghostOffset =
            self.ghostOffset
            + definition.speed
            * self.ghostDirection
            * delta

        if self.ghostOffset >= definition.distance then
            self.ghostOffset =
                definition.distance

            self.ghostDirection = -1
        elseif self.ghostOffset <= 0 then
            self.ghostOffset = 0
            self.ghostDirection = 1
        end

        x = x + self.ghostOffset
    end

    scene.SetComponent(
        self.ghostEntity,
        "transform",
        x,
        y
    )

    objects.UpdateGhost(
        self.ghostEntity,
        self.selectedItem
    )
end

function controller:UpdatePlacement(delta)
    if input.KeyPressed("1") then
        self.selectedItem = "spike"
        self:CreateGhost()
    end

    if input.KeyPressed("2") then
        self.selectedItem = "platform"
        self:CreateGhost()
    end

    self:UpdateGhost(delta)

    if not input.MousePressed() then
        return
    end

    local x, y =
        input.GetMousePosition()

    self:DestroyGhost()

    local entity =
        objects.Create(
            self.selectedItem,
            x,
            y
        )

    table.insert(
        self.placedObjects,
        {
            entity = entity,
            type = self.selectedItem,
            x = x,
            y = y
        }
    )

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

function controller:ClearPlacedObjects()
    for _, object in ipairs(self.placedObjects) do
        if object.entity ~= nil
            and scene.IsEntity(object.entity) then
            scene.RemoveEntity(object.entity)
        end
    end

    self.placedObjects = {}
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
        if scene.IsEntity(
                self.activePlayerEntity
            ) then
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

    if self.scores[1] > self.scores[2] then
        self.gameOverText =
        "Game Over - Player 1 Wins!"
    elseif self.scores[2] > self.scores[1] then
        self.gameOverText =
        "Game Over - Player 2 Wins!"
    else
        self.gameOverText =
        "Game Over - Draw!"
    end

    for i = 1, self.playerCount do
        print(
            "Player",
            i,
            "score:",
            self.scores[i]
        )
    end
end

function controller:SavePlacedObjects()
    levelIO.Save(
        self.placedObjects
    )
end

function controller:LoadPlacedObjects()
    self:ClearPlacedObjects()

    local loadedObjects =
        levelIO.Load()

    for _, object in ipairs(loadedObjects) do
        local entity =
            objects.Create(
                object.type,
                object.x,
                object.y
            )

        table.insert(
            self.placedObjects,
            {
                entity = entity,
                type = object.type,
                x = object.x,
                y = object.y
            }
        )
    end

    print("Level loaded")
end

function controller:OnUpdate(delta)
    while true do

        if input.KeyPressed("S") then
            self:SavePlacedObjects()
        end

        if input.KeyPressed("L") then
            self:LoadPlacedObjects()
        end

        self:UpdateUI()

        -- game phases: PLACEMENT, PLAYING, GAME_OVER
        if self.phase == "PLACEMENT" then
            self:UpdatePlacement(delta)
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
