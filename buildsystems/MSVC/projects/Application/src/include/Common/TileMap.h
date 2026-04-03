// TileMap.h
#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

namespace game
{
    class TileMap
    {
      public:
        using Tile = std::uint8_t;

        static constexpr Tile Empty = 0;
        static constexpr Tile Solid = 1;

        TileMap() = default;

        TileMap(int width, int height, int tileSize = 16) { Resize(width, height, tileSize); }

        void Resize(int width, int height, int tileSize = 16)
        {
            if (width <= 0 || height <= 0 || tileSize <= 0)
                throw std::invalid_argument("TileMap dimensions must be > 0");

            m_Width    = width;
            m_Height   = height;
            m_TileSize = tileSize;
            m_Tiles.assign(static_cast<size_t>(width * height), Empty);
        }

        void Clear(Tile value = Empty) { std::fill(m_Tiles.begin(), m_Tiles.end(), value); }

        [[nodiscard]] int Width() const { return m_Width; }
        [[nodiscard]] int Height() const { return m_Height; }
        [[nodiscard]] int TileSize() const { return m_TileSize; }

        [[nodiscard]] bool InBounds(int tx, int ty) const
        {
            return tx >= 0 && tx < m_Width && ty >= 0 && ty < m_Height;
        }

        [[nodiscard]] Tile Get(int tx, int ty) const
        {
            if (!InBounds(tx, ty))
                return Empty;

            return m_Tiles[Index(tx, ty)];
        }

        void Set(int tx, int ty, Tile value)
        {
            if (!InBounds(tx, ty))
                return;

            m_Tiles[Index(tx, ty)] = value;
        }

        [[nodiscard]] const std::vector<Tile>& Raw() const { return m_Tiles; }
        [[nodiscard]] std::vector<Tile>&       Raw() { return m_Tiles; }

      private:
        [[nodiscard]] int Index(int tx, int ty) const { return ty * m_Width + tx; }

      private:
        int               m_Width    = 0;
        int               m_Height   = 0;
        int               m_TileSize = 16;
        std::vector<Tile> m_Tiles;
    };
} // namespace game
