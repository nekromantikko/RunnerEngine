#ifndef TILES_H
#define TILES_H
#include <string>
#include "shared.h"
#include "bsp.h"

class Texture;

//TILESETS

enum TileType : u32
{
    ANY = 0,
    EMPTY = 0, // 0
    SOLID = 1, // 1
    PASS_THROUGH = 2, // 2
    JUMP_THROUGH = 4, // 3
    PASS_THROUGH_FLIP = 8, // 4
    LEDGE = 16, // 5
};

struct TileAnimFrame
{
    u32 frame;
    u32 duration;
};

struct TileAnim
{
    std::vector<TileAnimFrame> frames;
    u32 *tileIndex;
    /////////////////
    u32 currentFrame;
    r32 accumulator;
};

struct Tile
{
    TileType type;
    r32 slope;
    u32 mask[runnerTileSize];
    u32 index;
};

class Tileset
{
public:
    Tileset(std::string fname);
    ~Tileset();
    Texture *get_diffuse();
    Texture *get_lightmap();
    Texture *get_normal();
    memory_index get_size() const;
    Tile *get_tile(memory_index index);
    v2 get_coord(memory_index index);
    void load(std::string fname);
    void create_texcoords();
    void animate();
private:
    Texture *texture_, *lightmap_, *normal_;
    std::vector<Tile> tiles;
    std::vector<TileAnim> animations;
    std::vector<v2> texCoords;
};

struct TilesetAndGid
{
    Tileset *tileset;
    u32 gid;
};

struct TilesetGroup
{
    Tileset *tileset;
    TileBSPNode rootNode;
};

struct TileLayer
{
    u32 width;
    u32 height;
    r32 xScroll;
    r32 yScroll;
    bool32 xTiling;
    bool32 yTiling;
    u32 z;
    bool32 collision;

    u32 groupCount;
    TilesetGroup *tilesetGroups;

    Transform previous;
};

struct TileLayerInfo;

//This class keeps track of all the tile layers that exist, and also creates and destroys them
namespace TileManager
{
    //void load_tile_layer(rapidxml::xml_node<> *layer, glm::ivec2 levelDimensions, TilesetData &tileset);
    void create_layer(TileLayerInfo &info, u32 groupCount, TilesetGroup *groups);
    void create_layer_textures();
    void update();
    void update_textures(TileLayer *layer);
    void draw_layer(TileLayer *layer);
    void clear_layers();
    TileLayer *get_layers();
    u32 get_layer_amount();
    s32 get_tile_height(Tile *tile, s32 xInUnits);
}


#endif // TILES_H
