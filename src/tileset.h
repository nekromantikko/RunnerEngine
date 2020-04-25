#ifndef TILESET_H
#define TILESET_H
#include <string>
#include "shared.h"

class Texture;

//TILESETS

enum TileType : u16
{
    TILE_ANY = 0,
    TILE_EMPTY = 0,

    TILE_SOLID = 1,

    TILE_PASS_THROUGH = 2,
    TILE_PASS_THRU = 2,
    TILE_SLOPE = 2,

    TILE_JUMP_THROUGH = 3,
    TILE_JUMP_THRU = 3,

    TILE_PASS_THROUGH_FLIP = 4,
    TILE_PASS_THRU_FLIP = 4,
    TILE_SLOPE_FLIP = 4,

    TILE_LEDGE = 5,
};

//64 bit size (8 bytes)
struct Tile
{
    TileType type;
    r32 slope;
    u16 height[TILE_SIZE];
};

struct Tileset
{
    Tile tiles[256];
    Texture *baseColor;
    Texture *normalMap;
};

#endif // TILES_H
