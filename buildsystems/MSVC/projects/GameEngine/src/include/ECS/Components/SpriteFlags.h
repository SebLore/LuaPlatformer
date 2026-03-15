#pragma once

#include <cstdint>

namespace components
{

    enum class SpriteFlags : std::uint8_t
    {
        NONE   = 0,
        FLIP_X = 1u << 0,
        FLIP_Y = 1u << 1,
    };

    constexpr SpriteFlags operator|(SpriteFlags a, SpriteFlags b)
    {
        return static_cast<SpriteFlags>(
            static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
    }
    constexpr SpriteFlags operator&(SpriteFlags a, SpriteFlags b)
    {
        return static_cast<SpriteFlags>(
            static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
    }
    constexpr SpriteFlags& operator|=(SpriteFlags& a, SpriteFlags b)
    {
        a = a | b;
        return a;
    }
    constexpr SpriteFlags& operator&=(SpriteFlags& a, SpriteFlags b)
    {
        a = a & b;
        return a;
    }

    constexpr bool HasFlag(SpriteFlags value, SpriteFlags flag)
    {
        return (value & flag) != SpriteFlags::NONE;
    }
    constexpr void SetFlag(SpriteFlags& value, SpriteFlags flag, bool on)
    {
        if (on)
            value |= flag;
        else
            value = static_cast<SpriteFlags>(
                static_cast<std::uint8_t>(value) &
                ~static_cast<std::uint8_t>(flag));
    }
} // namespace components
