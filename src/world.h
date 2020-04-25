#ifndef WORLD_H
#define WORLD_H

#include "tileset.h"

#define TILE_CHUNK_WIDTH 16
#define TILE_CHUNK_HEIGHT 16

struct TileChunk
{
    u8 tile[TILE_CHUNK_WIDTH*TILE_CHUNK_HEIGHT];
};

struct TileLayer
{
    TileChunk *chunk;
    ivec2 *chunkOffset;
    u32 chunkCount;

    Texture *chunkTexture;
    u32 chunkTextureCount;

    v2 scroll;
    u32 z;
    bool32 xTiling;
    bool32 yTiling;
};

namespace World
{
    TileLayer *get_main_layer();
}

#endif // WORLD_H
