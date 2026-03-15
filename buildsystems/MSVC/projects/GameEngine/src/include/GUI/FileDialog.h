#pragma once

#include <raygui.h>

namespace gui
{
    // helper function for converting ints into a rectangle (floats)
    static constexpr Rectangle RectI(int x, int y, int w, int h)
    {
        return Rectangle{ static_cast<float>(x),
                          static_cast<float>(y),
                          static_cast<float>(w),
                          static_cast<float>(h) };
    }

    // file dialog context data
    struct GuiWindowFileDialogState
    {
        // window variables
        Rectangle Bounds       = { 0, 0, 0, 0 };
        Vector2   Offset       = { 0, 0 };
        bool      WindowActive = true;
        bool      Dragging     = false;
        bool      DragEnable   = false;

        char dirPathText[1024]     = { 0 };
        char fileNameText[256]     = { 0 };
        char dirPathTextCopy[1024] = { 0 };
        char fileNameTextCopy[256] = { 0 };
        char filterExt[32]         = { 0 };

        int prevFilesListActive;

        FilePathList dirFiles;
    };

    inline GuiWindowFileDialogState
    InitGuiWindowFileDialog(const char* initPath, int width, int height)
    {
        GuiWindowFileDialogState state = { false };

        state.Bounds = RectI(
            GetScreenWidth() / 2 - width / 2,
            GetScreenHeight() / 2 - height / 2,
            width,
            height);

        // Custom variables initialization
        if (initPath && DirectoryExists(initPath))
        {
            strcpy(state.dirPathText, initPath);
        }
        else if (initPath && FileExists(initPath))
        {
            strcpy(state.dirPathText, GetDirectoryPath(initPath));
            strcpy(state.fileNameText, GetFileName(initPath));
        }
        else
            strcpy(state.dirPathText, GetWorkingDirectory());

        // TODO: Why we keep a copy?
        strcpy(state.dirPathTextCopy, state.dirPathText);
        strcpy(state.fileNameTextCopy, state.fileNameText);

        state.filterExt[0] = '\0';
        //strcpy(state.filterExt, "all");

        state.dirFiles.count = 0;

        return state;
    }

} // namespace gui
