#pragma once

#include "AssetTags.h"
#include "IAssetCache.h"
#include "RaylibTraits.h" // for unloading assets properly

#include <unordered_map>
#include <utility> // for std::move

#include "raylib.h"

namespace assets
{
    /// Generic typed asset cache implementing IAssetCache<T>
    template <typename IdType, typename AssetType>
    class AssetCache : public IAssetCache<IdType, AssetType>
    {
      public:
        /// Default virtual destructor
        ~AssetCache() override { AssetCache::Clear(); };

        /// Tries to get a typed asset pointer from cache
        /// @return Pointer if found, otherwise nullptr
        AssetType* TryGet(IdType id) override
        {
            const auto it = m_Cache.find(id);
            return it != m_Cache.end() ? &it->second : nullptr;
        }

        /// Tries to get a const typed asset pointer from cache
        /// @return Pointer if found, otherwise nullptr
        const AssetType* TryGet(IdType id) const override
        {
            const auto it = m_Cache.find(id);
            return it != m_Cache.end() ? &it->second : nullptr;
        }

        /// Stores a typed asset using unique ownership
        void Cache(IdType id, AssetType asset) override
        {
            auto it = m_Cache.find(id);
            if (it != m_Cache.end())
            {
                RaylibTraits<AssetType>::Unload(it->second);
                it->second = std::move(asset);
            }
            else
            {
                m_Cache.emplace(id, std::move(asset));
            }
        }

        /// Clears all cached assets
        void Clear() override
        {
            for (auto& [id, asset] : m_Cache)
                RaylibTraits<AssetType>::Unload(asset);

            // safe to clear items after unloading all the memory
            m_Cache.clear();
        }

        ///  Enable iterating over all caches assets in case we need to do something with them (e.g. unload)
        auto begin() { return m_Cache.begin(); }
        auto end() { return m_Cache.end(); }
        auto begin() const { return m_Cache.begin(); }
        auto end() const { return m_Cache.end(); }

      private:
        std::unordered_map<IdType, AssetType> m_Cache;
    };

    /// Type aliases for specific asset caches
    using TextureCache = AssetCache<TextureId, Texture2D>;
    using ShaderCache  = AssetCache<ShaderId, Shader>;
    //using ImageCache   = AssetCache<ImageId, Image>;
    //using SoundCache   = AssetCache<SoundId, Sound>;
    //using MusicCache   = AssetCache<MusicId, Music>;
    /// TODO: add more as needed

} // namespace assets
