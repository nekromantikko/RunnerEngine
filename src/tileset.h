#ifndef TILESET_H
#define TILESET_H
/*
#define TILESET_MAX_SIZE 1024*1024

enum TileType : u32
{
    ANY = 0,
    EMPTY = 0,
    SOLID = 1,
    PASS_THROUGH = 2,
    JUMP_THROUGH = 3,
    PASS_THROUGH_FLIP = 4,
    LEDGE = 5,
};

struct TileCollisionData
{
    TileType type;
    r32 slope;
    u32 height[runnerTileSize];
};

struct Tileset
{
    TileCollisionData collisionData[TILESET_MAX_SIZE];
    //TODO: load textures and other assets on demand from file with name string
    //const char *textureName;

    //placeholder:
    Texture *texture_, *lightmap_, *normal_;
};*/



#endif // TILESET_H
