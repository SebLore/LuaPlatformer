#pragma once

#include <raylib.h> // for Texture2D

#include "AssetCache.h"
#include "AssetRegistry.h"
#include "AssetTags.h"
#include "utils.h"

// STL
#include <filesystem>

namespace assets
{
    namespace fs = std::filesystem;

    class AssetManager
    {
      public:
        struct Dirs
        {
            static inline auto ASSETS_DIR   = "assets";
            static inline auto TEXTURES_DIR = "assets/textures";

            fs::path root     = fs::current_path();
            fs::path assets   = utils::FindDirectory(ASSETS_DIR);
            fs::path textures = utils::FindDirectory(TEXTURES_DIR);

            void Print() const;
        };

      public:
        AssetManager();
        ~AssetManager() { Clear(); }

        void Clear();

        TextureId        RegisterTexture(std::string_view path);
        bool             LoadTextureFile(TextureId id);
        const Texture2D* TryGetTexture(TextureId id) const;

        // TODO: implement load and try get if I actually end up using shaders
        ShaderId RegisterShader(std::string_view path)
        {
            return m_Registry.RegisterShader(path);
        }

        AssetRegistry&       Registry() { return m_Registry; }
        const AssetRegistry& Registry() const { return m_Registry; }

      private:
        TextureCache m_Textures{};
        ShaderCache  m_Shaders{};

        AssetRegistry m_Registry{};
    };
} // namespace assets
