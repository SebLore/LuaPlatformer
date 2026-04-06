-- Editor.lua
local Editor = {}

function Editor:Init()
    self._editor = nil
end

function Editor:Open()
    if self._editor then return end
    self._editor = EditorUI()
end

function Editor:Close()
    if not self._editor then return end
    self._editor:Close()
    self._editor = nil
end

return Editor