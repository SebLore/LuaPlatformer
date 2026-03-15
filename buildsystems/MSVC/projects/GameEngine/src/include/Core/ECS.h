#pragma once

#include <entt/entt.hpp>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

struct lua_State; // forward-declare (include lua.hpp where needed)

namespace Core
{

    using Entity                       = entt::entity;
    static constexpr Entity NullEntity = entt::null;

    // ------------------------------------------------------------
    // ISystem: base interface for systems
    // ------------------------------------------------------------
    class ECS;

    struct ISystem
    {
        virtual ~ISystem() = default;
        virtual void Init(ECS&) {}
        virtual void Shutdown(ECS&) {}
        virtual void Update(ECS&, float /*dt*/) {}
        virtual void FixedUpdate(ECS&, float /*fixedDt*/) {}
    };

    // ------------------------------------------------------------
    // ECS: thin wrapper over entt::registry + systems list
    // ------------------------------------------------------------
    class ECS final
    {
      public:
        ECS() = default;

        ECS(const ECS&)            = delete;
        ECS& operator=(const ECS&) = delete;

        // Registry access
        entt::registry&       Registry() { return m_registry; }
        const entt::registry& Registry() const { return m_registry; }

        // Entity lifecycle
        Entity CreateEntity() { return m_registry.create(); }

        void DestroyEntity(Entity e)
        {
            if (m_registry.valid(e))
                m_registry.destroy(e);
        }

        bool Valid(Entity e) const { return m_registry.valid(e); }

        static std::uint32_t ToId(Entity e)
        {
            return static_cast<std::uint32_t>(entt::to_integral(e));
        }

        // Component helpers
        template <class T, class... Args> T& Add(Entity e, Args&&... args)
        {
            static_assert(
                std::is_same_v<T, std::decay_t<T>>,
                "Component type must be a plain type.");
            return m_registry.emplace<T>(e, std::forward<Args>(args)...);
        }

        template <class T> T& Get(Entity e) { return m_registry.get<T>(e); }

        template <class T> const T& Get(Entity e) const
        {
            return m_registry.get<T>(e);
        }

        template <class T> bool Has(Entity e) const
        {
            return m_registry.any_of<T>(e);
        }

        template <class T> void Remove(Entity e) { m_registry.remove<T>(e); }

        template <class... Ts> auto View() { return m_registry.view<Ts...>(); }

        template <class... Ts> auto View() const
        {
            return m_registry.view<Ts...>();
        }

        // Systems
        template <class TSystem, class... Args>
        TSystem& AddSystem(Args&&... args)
        {
            static_assert(
                std::is_base_of_v<ISystem, TSystem>,
                "System must derive from ISystem.");
            auto ptr = std::make_unique<TSystem>(std::forward<Args>(args)...);
            TSystem& ref = *ptr;
            m_systems.emplace_back(std::move(ptr));
            return ref;
        }

        void InitSystems()
        {
            for (auto& s : m_systems)
                s->Init(*this);
        }

        void ShutdownSystems()
        {
            for (auto& s : m_systems)
                s->Shutdown(*this);
        }

        void UpdateSystems(float dt)
        {
            for (auto& s : m_systems)
                s->Update(*this, dt);
        }

        void FixedUpdateSystems(float fixedDt)
        {
            for (auto& s : m_systems)
                s->FixedUpdate(*this, fixedDt);
        }

        // --------------------------------------------------------
        // Lua: optional attachment point (no binding library assumed)
        // --------------------------------------------------------
        void       AttachLua(lua_State* L) { m_lua = L; }
        lua_State* Lua() const { return m_lua; }

      private:
        entt::registry                        m_registry{};
        std::vector<std::unique_ptr<ISystem>> m_systems{};
        lua_State* m_lua = nullptr; // owned elsewhere
    };

} // namespace Core
