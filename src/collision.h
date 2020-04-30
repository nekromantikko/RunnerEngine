#ifndef COLLISION_H
#define COLLISION_H

#include "tileset.h"
#include "vector.h"
#include "math.h"
#include <vector>

enum CollisionFlag
{
    COLLISION_NONE = 0,
    COLLISION_PLAYER = 1,
    COLLISION_ENEMY = 2,
    COLLISION_SPIKES = 4,
    COLLISION_FOLIAGE = 8,
    COLLISION_WALL = 16,
};

struct TileHit
{
    bool32 hit;
    Tile tile;
    fvec2 pos;
};

class Entity;

struct EntityHit
{
    bool hit;
    Entity *entity;
};

struct RaycastHit
{
    bool hit;
    u32 length;
    fvec2 pos;
};

//collision functions return first collision found, whether or not it is the closest
//this pretty much requires two collision checks, one for horizontal and another for vertical movement
//which is fine though
namespace Collision
{
    //tile type 0 (empty) could be any type? since no collision can happen with empty tiles anyway
    TileHit box_tile_collision(Rectangle2 rect, TileType type);
    //returns all tiles
    TileHit *box_tile_collision_multiple(Rectangle2 rect, TileType type, int &count);

    bool separating_axis_rects(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 ba, fvec2 bb, fvec2 bc, fvec2 bd);
    bool separating_axis_rect_circle(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 cPos, u32 radius);
    bool separating_axis_rect_point(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 pPos);
    bool rect_point_collision(Rectangle2 rect, fvec2 pPos);
    bool circle_point_collision(v2 cPos, u32 radius, fvec2 pPos);

    bool rect_rect_collision(Rectangle2 a, Rectangle2 b);

    std::vector<fvec2> bresenham_line(fvec2 a, fvec2 b);
    bool tile_point_free(Tile tile, fvec2 relativePos);
    bool collision_tile_mask(fvec2 position);
    RaycastHit cast_ray(fvec2 position, v2 direction, r32 lengthInPx);
}


#endif // COLLISION_H
