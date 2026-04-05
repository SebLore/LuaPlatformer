// TileMap.h
#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

namespace game
{
    enum class TileType : std::uint8_t
    {
        Empty = 0,
        Solid = 1
    };



    class TileMap
    {
    public:
        TileMap() = default;

        /// @brief Create a new tile map with the specified dimensions and tile size.
        /// @param width The width of the tile map in tiles.
        /// @param height The height of the tile map in tiles.
        /// @param tileSize The dimensions of each tile in pixels.
        TileMap(int width, int height, int tileSize = 16) { Resize(width, height, tileSize); }

        void Resize(int width, int height, int tileSize = 16)
        {
            if (width <= 0 || height <= 0 || tileSize <= 0)
                throw std::invalid_argument("TileMap dimensions must be > 0");

            m_Width = width;
            m_Height = height;
            m_TileSize = tileSize;
            m_Tiles.assign(static_cast<size_t>(width * height), TileType::Empty);
        }


        // use ranges if version >= c++20

        void Clear(TileType value = TileType::Empty)
        {
            std::fill(m_Tiles.begin(), m_Tiles.end(), value);
        }

        [[nodiscard]] int Width() const { return m_Width; }
        [[nodiscard]] int Height() const { return m_Height; }
        [[nodiscard]] int TileSize() const { return m_TileSize; }

        [[nodiscard]] bool InBounds(int tx, int ty) const
        {
            return tx >= 0 && tx < m_Width && ty >= 0 && ty < m_Height;
        }

        [[nodiscard]] TileType Get(int tx, int ty) const
        {
            if (!InBounds(tx, ty))
                return TileType::Empty;

            return m_Tiles[Index(tx, ty)];
        }

        void Set(int tx, int ty, TileType value)
        {
            if (!InBounds(tx, ty))
                return;

            m_Tiles[Index(tx, ty)] = value;
        }

        [[nodiscard]] const std::vector<TileType>& Raw() const { return m_Tiles; }
        [[nodiscard]] std::vector<TileType>& Raw() { return m_Tiles; }

    private:
        [[nodiscard]] int Index(int tx, int ty) const { return ty * m_Width + tx; }

    private:
        int               m_Width = 0;
        int               m_Height = 0;
        int               m_TileSize = 16;
        std::vector<TileType> m_Tiles;
    };
} // namespace game
