
local levelIO = {}

local savePath = "level_save.txt"


function levelIO.Save(objects)
    local file = io.open(savePath, "w")

    if file == nil then
        print("Failed to open save file")
        return
    end

    for _, object in ipairs(objects) do
        file:write(
            object.type,
            " ",
            object.x,
            " ",
            object.y,
            "\n"
        )
    end

    file:close()

    print("Level saved")
end

function levelIO.Load()
    local objects = {}

    local file = io.open(savePath, "r")

    -- No save yet is perfectly valid.
    if file == nil then
        print("No level save found")
        return objects
    end

    for line in file:lines() do
        local objectType, x, y =
            line:match(
                "(%S+)%s+(%-?[%d%.]+)%s+(%-?[%d%.]+)"
            )

        if objectType ~= nil then
            table.insert(
                objects,
                {
                    type = objectType,
                    x = tonumber(x),
                    y = tonumber(y)
                }
            )
        end
    end

    file:close()

    print(
        "Loaded",
        #objects,
        "placed objects"
    )

    return objects
end

return levelIO
