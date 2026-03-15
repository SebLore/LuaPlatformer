#pragma once

#include <raylib.h>

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <entt/entt.hpp>

#include "Caching/TextureCache.h"
#include "ECS/Systems/ISystem.h"

class Engine
{
    struct Config
    {
        std::string BasePath = "../../assets/";

        TextureCache::Config TextureCacheConfig{
            .MaxCount = 200,
            .BasePath = Config::BasePath + "textures/",
        };
    };

  public:
    using Systems = std::vector<std::unique_ptr<ISystem>>;

    Engine() = default;
    ~Engine();

    explicit Engine(const Config& config);
    void Initialize(const Config& config);

    // --------------------------------------------------------------------------------------------
    // Resources
    // --------------------------------------------------------------------------------------------

    Texture2D* LoadTexture(std::string_view fileName, int flags = 0);
    void       UnloadTexture(std::string_view fileName);

    // --------------------------------------------------------------------------------------------
    // ECS
    // --------------------------------------------------------------------------------------------

    entt::registry& GetRegistry() { return m_Registry; }

    // --| ECS Systems |---------------------------------------------------------------------------
    template <class TSystem, class... Args>
    static TSystem& AddSystem(Systems& out, Args&&... args)
    {
        auto sys = std::make_unique<TSystem>(std::forward<Args>(args)...);

        TSystem& ref = *sys;
        out.emplace_back(std::move(sys));

        return ref;
    }

    template <class TSystem, class... Args> TSystem& AddSystem(Args&&... args)
    {
        return AddSystem<TSystem>(m_Systems, std::forward<Args>(args)...);
    }

    Systems&       GetSystems() { return m_Systems; }
    const Systems& GetSystems() const { return m_Systems; }

    // --| ECS Components |------------------------------------------------------------------------
    template <class TComponent, class... Args>
    TComponent& RegisterComponent(entt::entity e, Args&&... args)
    {
        assert(m_Registry.valid(e) && "RegisterComponent: invalid entity");

        return m_Registry.emplace_or_replace<TComponent>(
            e,
            std::forward<Args>(args)...);
    }

  private:
    std::string ToAssetTexturePath(std::string_view fileName) const;

    void CleanUp();

  private:
    // -- Assets TODO: move to a resource manager later --------------------------
    std::string m_AssetsPath = "../../assets/";

    // > Textures

    static constexpr size_t MAX_TEXTURE_COUNT = 200;
    TextureCache            m_TextureCache{ { MAX_TEXTURE_COUNT,
                                              m_AssetsPath + "textures/" } };

    // > Sprites

    // -- ECS -------------------------------------------------------------------------------------
    entt::registry m_Registry;
    // > Systems
    Systems m_Systems;
};
