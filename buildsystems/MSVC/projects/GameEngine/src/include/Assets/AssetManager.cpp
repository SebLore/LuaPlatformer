
#include "AssetManager.h"

#include <iostream>
#include <ranges>

#include "utility/fileutils.h"

namespace assets
{
    void AssetManager::Dirs::Print() const
    {
        std::cout << "Root: " << root.string() << "\n";
        std::cout << "Assets: " << assets.string() << "\n";
        std::cout << "Textures: " << textures.string() << "\n";
    }

    AssetManager::AssetManager()
    {
        auto dirs       = m_Registry.GetDirs();
        dirs.assetsRoot = util::FindDirectory("assets");
        dirs.binRoot    = fs::current_path();
        dirs.textures   = "textures";
        dirs.shaders    = "shaders";

        m_Registry.SetDirs(dirs);
    }

    void AssetManager::Clear()
    {
        m_Textures.Clear();

        m_Registry.Clear();
    }

    TextureId AssetManager::RegisterTexture(std::string_view path)
    {
        return m_Registry.RegisterTexture(path);
    }

    bool AssetManager::LoadTextureFile(TextureId id)
    {
        if (m_Textures.TryGet(id))
            return true; // already loaded

        const auto& path = m_Registry.GetFullPath(id);

        const Texture2D texture = LoadTexture(path.string().c_str());

        if (texture.id == 0)
        {
            std::cerr << "Failed to load texture: " << path.string() << "\n";
            return false;
        }

        m_Textures.Cache(id, texture);

        return true;
    }

    const Texture* AssetManager::TryGetTexture(TextureId id) const
    {
        return m_Textures.TryGet(id);
    }
} // namespace assets
