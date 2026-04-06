-- Level.lua
-- Defines the Level class, which represents a game level and manages its state.
local Level = {}

function Level:Init()
    self._entities = {}
    self._isLoaded = false
end

function Level:Load()
    -- Load level resources and initialize entities
    self._isLoaded = true
end

function Level:Update(dt)
    if not self._isLoaded then return end
    -- Update all entities in the level
    for _, entity in ipairs(self._entities) do
        entity:Update(dt)
    end
end