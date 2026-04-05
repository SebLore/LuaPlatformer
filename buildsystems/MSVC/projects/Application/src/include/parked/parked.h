#pragma once

#include <LuaWrapper.h>
#include "LuaBindings.h"

#ifdef USE_META // meta support for reflection
#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp> // meta_factory
#include <entt/meta/meta.hpp>
#endif

// -- tiny helpers to call Lua functions safely --
static void register_meta()
{
#ifdef USE_META
    using namespace entt::literals;

    entt::meta_factory<Position>{}.type("Position"_hs).data<&Position::x>("x"_hs).data<&Position::y>("y"_hs);

    entt::meta_factory<Circle>{}.type("Circle"_hs).data<&Circle::r>("r"_hs);
#endif
}
