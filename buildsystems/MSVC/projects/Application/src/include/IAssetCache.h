#pragma once

#include "AssetId.h"

namespace assets
{
    /// @brief Typed base interface for asset caches
    template <typename IdType, typename AssetType> class IAssetCache
    {
      public:
        virtual ~IAssetCache() = default;

        /// Tries to get a cached asset
        /// @return Pointer if found, otherwise nullptr
        virtual AssetType* TryGet(IdType id) = 0;

        /// Tries to get a cached asset (const overload)
        /// @return Pointer if found, otherwise nullptr
        virtual const AssetType* TryGet(IdType id) const = 0;

        /// @brief Caches a loaded asset and transfers ownership
        virtual void Cache(IdType id, AssetType asset) = 0;

        /// @brief Clears all cached assets
        virtual void Clear() = 0;
    };
} // namespace assets
