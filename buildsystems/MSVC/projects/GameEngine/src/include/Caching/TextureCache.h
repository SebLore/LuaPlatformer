#pragma once

#include <raylib.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LRUCache.h"

class TextureCache
{
  public:
    struct Config
    {
        size_t      MaxCount = 200;
        std::string BasePath = "../../assets/";
    };

  public:
    TextureCache() = default;
    ~TextureCache();
    explicit TextureCache(const Config& config);
    void Initialize(const Config& config);

    // Get cached texture or load it. Touches it in the LRU.
    // Returns nullptr if loading fails.
    Texture2D* Load(std::string_view fileName);

    // Unload a specific cached texture (no-op if not found).
    void Unload(std::string_view fileName);

    // unload all loaded textures
    void Clear();

    // set new base path
    void SetBasePath(std::string basePath);

    size_t Capacity() const { return m_Slots.size(); }
    size_t Count() const { return m_Map.size(); }

    // don't copy or move this, only ever create explicitly
    TextureCache(const TextureCache&)            = delete;
    TextureCache& operator=(const TextureCache&) = delete;
    TextureCache(TextureCache&&)                 = delete;
    TextureCache& operator=(TextureCache&&)      = delete;

  public:
    // Cache slot structure, to enable LRU
    static constexpr size_t npos = static_cast<size_t>(-1);
    struct Slot
    {
        Texture2D   tex{};
        std::string key;         // owns the map key string
        size_t      prev = npos; // LRU list
        size_t      next = npos;
        bool        used = false;
    };

    // C++20 heterogeneous lookup: find() with std::string_view without allocating
    struct StringHash
    {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept
        {
            return std::hash<std::string_view>{}(sv);
        }
        size_t operator()(const std::string& s) const noexcept
        {
            return (*this)(std::string_view{ s });
        }
    };
    struct StringEq
    {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const noexcept
        {
            return a == b;
        }

        bool
        operator()(const std::string& a, const std::string& b) const noexcept
        {
            return a == b;
        }
    };
    using Map = std::unordered_map<std::string, size_t, StringHash, StringEq>;

  private:
    std::string MakePath(std::string_view fileName) const;

    // LRU helpers
    void Touch(size_t idx);
    void AttachFront(size_t idx);
    void Detach(size_t idx);

    void   EvictOneLRU();  // evict tail
    size_t AllocateSlot(); // returns index or npos
    void   FreeSlot(size_t idx);

  private:
    std::vector<Slot> m_Slots;
    Map               m_Map;

    // freelist stack of indices
    std::vector<size_t> m_Free;

    // LRU head/tail indices
    size_t m_Head = npos;
    size_t m_Tail = npos;

    std::string m_BasePath;
};
