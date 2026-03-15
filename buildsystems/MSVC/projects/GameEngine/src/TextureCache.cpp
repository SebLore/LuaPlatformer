#include "Caching/TextureCache.h"

#include <utility> // for std::move

TextureCache::TextureCache(const Config& config)
    : m_Slots(config.MaxCount), m_BasePath(config.BasePath)
{
    m_Map.reserve(config.MaxCount);

    m_Free.reserve(config.MaxCount);
    for (size_t i = 0; i < config.MaxCount; ++i)
        m_Free.push_back((config.MaxCount - 1) - i);
}

void TextureCache::Initialize(const Config& config)
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

TextureCache::~TextureCache() { Clear(); }

void TextureCache::SetBasePath(std::string basePath)
{
    m_BasePath = std::move(basePath);
}

std::string TextureCache::MakePath(std::string_view fileName) const
{
    std::string path = m_BasePath;

    if (!path.empty())
        if (char last = path.back(); last != '/' && last != '\\')
            path.push_back('/');

    path.append(fileName.data(), fileName.size());

    return path;
}

Texture2D* TextureCache::Load(std::string_view fileName)
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
    Texture2D         tex  = ::LoadTexture(path.c_str());
    if (tex.id == 0)
    {
        FreeSlot(idx);
        return nullptr;
    }

    // 4) store into slot
    Slot& s = m_Slots[idx];
    s.tex   = tex;
    s.key.assign(fileName.data(), fileName.size());
    s.used = true;

    // 5) map key -> slot
    m_Map.emplace(s.key, idx);

    // 6) mark as MRU (most recently used)
    AttachFront(idx);

    return &s.tex;
}

/// @brief Unload all textures and reset the cache.
void TextureCache::Clear()
{
    for (auto& s : m_Slots)
    {
        if (s.used && s.tex.id != 0)
            ::UnloadTexture(s.tex);
        s = Slot{};
    }

    m_Map.clear();

    m_Free.clear();
    m_Free.reserve(m_Slots.size());
    for (size_t i = 0; i < m_Slots.size(); ++i)
        m_Free.push_back((m_Slots.size() - 1) - i);

    m_Head = npos;
    m_Tail = npos;
}

void TextureCache::Unload(std::string_view fileName)
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

void TextureCache::Touch(size_t idx)
{
    if (m_Head == idx)
        return;

    Detach(idx);
    AttachFront(idx);
}

void TextureCache::AttachFront(size_t idx)
{
    Slot& slot = m_Slots[idx];
    slot.prev  = npos;
    slot.next  = m_Head;

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

void TextureCache::Detach(size_t idx)
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

void TextureCache::EvictOneLRU()
{
    // no op if no elements
    if (m_Tail == npos)
        return;

    const size_t idx = m_Tail;
    Slot&        s   = m_Slots[idx];

    Detach(idx);

    if (!s.key.empty())
        m_Map.erase(s.key);

    if (s.tex.id != 0)
        ::UnloadTexture(s.tex);

    s = Slot{};
    FreeSlot(idx);
}

size_t TextureCache::AllocateSlot()
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

void TextureCache::FreeSlot(size_t idx) { m_Free.push_back(idx); }
