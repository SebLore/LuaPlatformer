#pragma once

// TODO: use this in TextureCache
#if 0
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <utility>

template<class T>
class LRUCache
{
public:
    LRUCache() = default;
    ~LRUCache() = default;

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
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;

public:
    // Cache slot structure, to enable LRU
    static constexpr size_t npos = static_cast<size_t>(-1);
    struct Slot
    {
        T           element{};
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
        /*       bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
               bool operator()(const std::string& a, std::string_view b) const noexcept { return std::string_view{ a } == b; }
               bool operator()(std::string_view a, const std::string& b) const noexcept { return a == std::string_view{ b }; }*/
    };
    using Map = std::unordered_map<std::string, size_t, StringHash, StringEq>;

private:
    virtual std::string MakePath(std::string_view fileName) const;

    // LRU helpers
    virtual void Touch(size_t idx);
    virtual void AttachFront(size_t idx);
    virtual void Detach(size_t idx);

    virtual void   EvictOneLRU();  // evict tail
    virtual size_t AllocateSlot(); // returns index or npos
    virtual void   FreeSlot(size_t idx);

protected:
    std::vector<Slot> m_Slots;
    Map               m_Map;

    // freelist stack of indices
    std::vector<size_t> m_Free;

    // LRU head/tail indices
    size_t m_Head = npos;
    size_t m_Tail = npos;

    std::string m_BasePath;
};

LRUCache::LRUCache(const Config& config)
    : m_Slots(config.MaxCount), m_BasePath(config.BasePath)
{
    m_Map.reserve(config.MaxCount);

    m_Free.reserve(config.MaxCount);
    for (size_t i = 0; i < config.MaxCount; ++i)
        m_Free.push_back((config.MaxCount - 1) - i);
}

void LRUCache::Initialize(const Config& config)
{
    m_Slots.resize(config.MaxCount);
    m_BasePath = config.BasePath;

    m_Map.clear();
    m_Map.reserve(config.MaxCount);

    m_Free.clear();
    m_Free.reserve(config.MaxCount);
    for (size_t i = 0; i < config.MaxCount; ++i)
        m_Free.push_back((config.MaxCount - 1) - i);

    m_Head = npos;
    m_Tail = npos;
}

LRUCache::~LRUCache() { Clear(); }

void LRUCache::SetBasePath(std::string basePath)
{
    m_BasePath = std::move(basePath);
}

std::string LRUCache::MakePath(std::string_view fileName) const
{
    std::string path = m_BasePath;

    if (!path.empty())
        if (char last = path.back(); last != '/' && last != '\\')
            path.push_back('/');

    path.append(fileName.data(), fileName.size());

    return path;
}

Texture2D* LRUCache::Load(std::string_view fileName)
{
    if (auto it = m_Map.find(fileName); it != m_Map.end())
    {
        size_t idx = it->second;
        Touch(idx);
        return &m_Slots[idx].tex;
    }

    // 2) allocate a slot (evict if needed)
    const size_t idx = AllocateSlot();
    if (idx == npos)
        return nullptr;

    // 3) load from disk
    const std::string path = MakePath(fileName);
    Texture2D         tex = ::LoadTexture(path.c_str());
    if (tex.id == 0)
    {
        FreeSlot(idx);
        return nullptr;
    }

    // 4) store into slot
    Slot& s = m_Slots[idx];
    s.tex = tex;
    s.key.assign(fileName.data(), fileName.size());
    s.used = true;

    // 5) map key -> slot
    m_Map.emplace(s.key, idx);

    // 6) mark as MRU (most recently used)
    AttachFront(idx);

    return &s.tex;
}

/// @brief Unload all textures and reset the cache.
void LRUCache::Clear()
{


    m_Map.clear();

    m_Free.clear();
    m_Free.reserve(m_Slots.size());
    for (size_t i = 0; i < m_Slots.size(); ++i)
        m_Free.push_back((m_Slots.size() - 1) - i);

    m_Head = npos;
    m_Tail = npos;
}

void LRUCache::Unload(std::string_view fileName)
{
    auto it = m_Map.find(fileName);

    if (it == m_Map.end())
        return;

    const size_t idx = it->second;
    m_Map.erase(it);

    Slot& s = m_Slots[idx];
    Detach(idx);

    if (s.tex.id != 0)
        ::UnloadTexture(s.tex);

    s = Slot{};
    FreeSlot(idx);
}

void LRUCache::Touch(size_t idx)
{
    if (m_Head == idx)
        return;

    Detach(idx);
    AttachFront(idx);
}

void LRUCache::AttachFront(size_t idx)
{
    Slot& slot = m_Slots[idx];
    slot.prev = npos;
    slot.next = m_Head;

    if (m_Head != npos)
    {
        m_Slots[m_Head].prev = idx;
    }
    else
    {
        // list was empty
        m_Head = idx;
    }

    m_Head = idx;
}

void LRUCache::Detach(size_t idx)
{
    Slot& slot = m_Slots[idx];

    if (!slot.used)
        return; // not in use => should not be in LRU

    const size_t p = slot.prev;
    const size_t n = slot.next;

    if (p != npos)
        m_Slots[p].next = n;
    else
        m_Head = n;

    if (n != npos)
        m_Slots[n].prev = p;
    else
        m_Tail = p;

    slot.prev = npos;
    slot.next = npos;
}

void LRUCache::EvictOneLRU()
{
    // no op if no elements
    if (m_Tail == npos)
        return;

    const size_t idx = m_Tail;
    Slot& s = m_Slots[idx];

    Detach(idx);

    if (!s.key.empty())
        m_Map.erase(s.key);

    if (s.tex.id != 0)
        ::UnloadTexture(s.tex);

    s = Slot{};
    FreeSlot(idx);
}

size_t LRUCache::AllocateSlot()
{
    // first check: make sure there's an empty slot
    if (m_Free.empty())
        EvictOneLRU();

    // second check: if
    if (m_Free.empty())
        return npos;

    const size_t idx = m_Free.back();
    m_Free.pop_back();
    return idx;
}

void LRUCache::FreeSlot(size_t idx) { m_Free.push_back(idx); }
#endif