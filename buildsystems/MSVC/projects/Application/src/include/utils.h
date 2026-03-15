#pragma once

#include <filesystem>

namespace utils
{
    namespace fs = std::filesystem;

    inline fs::path
    FindDirectory(const fs::path& directoryName, std::size_t maxLevelsUp = 4)
    {
        if (directoryName.empty())
            throw std::invalid_argument("Directory name cannot be empty");

        fs::path currentPath = fs::current_path();

        for (std::size_t level = 0;; ++level)
        {
            fs::path candidate = currentPath / directoryName;

            if (fs::exists(candidate) && fs::is_directory(candidate))
                return candidate;

            if (level >= maxLevelsUp)
                break;

            fs::path parent = currentPath.parent_path();
            if (parent == currentPath)
                break; // reached filesystem root

            currentPath = parent;
        }

        throw std::runtime_error(
            "Directory \"" + directoryName.string() +
            "\" not found in current directory or within " +
            std::to_string(maxLevelsUp) + " parent level(s)");
    }
} // namespace utils
