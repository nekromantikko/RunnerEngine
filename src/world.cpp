#include "world.h"

#define MAX_TILE_CHUNK_COUNT 1024
#define MAX_TILE_LAYER_COUNT 8

namespace World
{
    //~2MB
    //TileChunk chunkData[MAX_TILE_CHUNK_COUNT];
    //ivec2 chunkOffsetData[MAX_TILE_CHUNK_COUNT];

    //Currently loaded tileset
    Tileset tileset;

    TileLayer mainLayer;

}

TileLayer *World::get_main_layer()
{
    return &mainLayer;
}
