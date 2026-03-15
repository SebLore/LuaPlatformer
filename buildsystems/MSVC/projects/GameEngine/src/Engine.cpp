#include "Engine.h"

#include <myutils.h>

#include <filesystem>
#include <iostream>

// -- Construction / Destruction ----------------------------------------------------------------

/// @brief Constructor
/// @param config Engine::Config struct
Engine::Engine(const Config& config)
    : m_AssetsPath(config.BasePath),
      m_TextureCache(config.TextureCacheConfig)
{
}

/// @brief Initializes the engine with the specified configuration, prefer explicit constructor.
/// @param config The configuration object containing initialization settings for the engine.
void Engine::Initialize(const Config& config)
{
    m_AssetsPath = config.BasePath;
    m_TextureCache.Initialize(config.TextureCacheConfig);
}

/// @brief Release all initialized Textures and anything else.
Engine::~Engine() { CleanUp(); }

// -- Helpers -------------------------------------------------------------------------------------

/// @brief Helper. Convert a file name to an asset texture path.
std::string Engine::ToAssetTexturePath(std::string_view file) const
{
    return m_AssetsPath + "textures/" + file.data();
}

/// @brief Release all resources.
void Engine::CleanUp() { m_TextureCache.Clear(); }

// -- Resources -----------------------------------------------------------------------------------

/// @brief Load an image from a file, store the image in memory and return a texture of it.
/// @param fileName name of the file, e.g. "player_sprite.png"
/// @param flags texture options, i.e. mipmaps, uv wrap, etc.
/// @return texture of the image readable by the GPU
Texture* Engine::LoadTexture(std::string_view fileName, int flags)
{
    return m_TextureCache.Load(fileName);

    // TODO: implement flags handling
#if 0
    if (auto i = utils::UmapFindPtr(m_TextureIDMap, fileName))
        return &m_TextureCache[*i];

    std::string path = ToAssetTexturePath(fileName);

    std::cout << "Loading texture from absolute path: "
        << fs::absolute(fs::path(path)) << '\n';
    Texture t = ::LoadTexture(path.c_str());
    if (t.id > 0)
    {
        // load succeeded, add to cache and return ptr
        size_t idx = m_TextureCache.size() - 1;
        m_TextureIDMap[fileName] = idx;
        m_TextureCache[idx] = std::move(t);
        return &m_TextureCache[idx];
    }
#endif
    // load failed
}

void Engine::UnloadTexture(std::string_view fileName)
{
    m_TextureCache.Unload(fileName);
}
