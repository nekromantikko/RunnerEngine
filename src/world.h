#ifndef WORLD_H
#define WORLD_H

#include "tileset.h"

#define TILE_CHUNK_WIDTH 16
#define TILE_CHUNK_HEIGHT 16

#define CHUNKS_PER_TEXTURE 256

struct TileChunk
{
    u8 tile[TILE_CHUNK_WIDTH*TILE_CHUNK_HEIGHT];
};

struct TileLayer
{
    TileChunk *chunk = NULL;
    ivec2 *chunkOffset = NULL;
    u32 chunkCount = 0;

    Texture *chunkTexture = NULL;
    u32 chunkTextureCount = 0;

    v2 scroll = {1.0f,1.0f};
    u32 z = 0;
    bool32 xTiling = false;
    bool32 yTiling = false;
};

namespace World
{
    TileLayer *get_main_layer();
    Tileset *get_tileset();

    void create_index_maps();
    void clear_index_maps();

    void update();

    bool load_tileset(const char *fname);
    void unload_tileset();
    bool load_map(const char *fname);
    void unload_map();
}

#endif // WORLD_H
