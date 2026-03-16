#pragma once

#include <raylib.h>

// defining type traits for different raylib assets for the asset manager
namespace assets
{
    template <typename T> struct RaylibTraits;

    template <> struct RaylibTraits<Texture2D>
    {
        static void Unload(Texture2D& asset)
        {
            ::UnloadTexture(asset);
            asset = {};
        }
    };

    // maybe using this later
    template <> struct RaylibTraits<Shader>
    {
        static void Unload(Shader& asset)
        {
            if (asset.id != 0)
            {
                ::UnloadShader(asset);
                asset = {};
            }
        }
    };

    template <> struct RaylibTraits<Image>
    {
        static void Unload(Image& asset)
        {
            if (asset.data != nullptr)
            {
                ::UnloadImage(asset);
                asset = {};
            }
        }
    };

    template <> struct RaylibTraits<Sound>
    {
        static void Unload(Sound& asset)
        {
            if (asset.frameCount > 0)
            {
                ::UnloadSound(asset);
                asset = {};
            }
        }
    };

    template <> struct RaylibTraits<Music>
    {
        static void Unload(Music& asset)
        {
            if (asset.ctxData != nullptr)
            {
                ::UnloadMusicStream(asset);
                asset = {};
            }
        }
    };
} // namespace assets
