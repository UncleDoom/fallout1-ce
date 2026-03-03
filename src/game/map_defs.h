#pragma once


namespace fallout {

constexpr int ELEVATION_COUNT = 3;

constexpr int SQUARE_GRID_WIDTH = 100;
constexpr int SQUARE_GRID_HEIGHT = 100;
constexpr int SQUARE_GRID_SIZE = SQUARE_GRID_WIDTH * SQUARE_GRID_HEIGHT;

constexpr int HEX_GRID_WIDTH = 200;
constexpr int HEX_GRID_HEIGHT = 200;
constexpr int HEX_GRID_SIZE = HEX_GRID_WIDTH * HEX_GRID_HEIGHT;

static inline bool elevationIsValid(int elevation)
{
    return elevation >= 0 && elevation < ELEVATION_COUNT;
}

static inline bool squareGridTileIsValid(int tile)
{
    return tile >= 0 && tile < SQUARE_GRID_SIZE;
}

static inline bool hexGridTileIsValid(int tile)
{
    return tile >= 0 && tile < HEX_GRID_SIZE;
}

} // namespace fallout
