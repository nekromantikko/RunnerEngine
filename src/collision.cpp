#include "collision.h"
#include "units.h"
#include "renderer.h"
#include "world.h"


TileHit Collision::box_tile_collision(Rectangle2 rect, TileType type)
{
    TileLayer *layer = World::get_main_layer();

    TileHit result;
    result.hit = false;

    Rectangle2 boxInTileSpace = rect;
    boxInTileSpace.x1 /= TILE_SIZE;
    boxInTileSpace.x2 /= TILE_SIZE;
    boxInTileSpace.y1 /= TILE_SIZE;
    boxInTileSpace.y2 /= TILE_SIZE;

    //find out how many chunks the box is intersecting with and store their indices
    u32 chunkCount = layer->chunkCount;
    u32 overlappingChunkCount = 0;
    u32 overlappingChunks[4];
    for (u32 i = 0; i < chunkCount, overlappingChunkCount < 4; i++)
    {
        fvec2 chunkOffset = layer->chunkOffset[i];
        Rectangle2 layerRect = {chunkOffset.x, chunkOffset.x + TILE_CHUNK_WIDTH, chunkOffset.y, chunkOffset.y + TILE_CHUNK_HEIGHT};

        if (rect_rect_collision(boxInTileSpace, layerRect))
        {
            overlappingChunks[overlappingChunkCount] = i;
            overlappingChunkCount++;
        }

    }

    if (overlappingChunkCount != 0)
    {
        Tileset *tileset = World::get_tileset();

        for (u32 i = 0; i < overlappingChunkCount, result.hit == false; i++)
        {
            fvec2 chunkOffset = layer->chunkOffset[overlappingChunks[i]];

            //transform into chunk space and clip box to fit inside chunk
            s32 left = boxInTileSpace.x1 - chunkOffset.x;
            s32 right = boxInTileSpace.x2 - chunkOffset.x;
            s32 top = boxInTileSpace.y1 - chunkOffset.y;
            s32 bottom = boxInTileSpace.y2 - chunkOffset.y;

            if (left < 0)
            {
                if (right < 0)
                    continue;
                else left = 0;
            }
            if (right > TILE_CHUNK_WIDTH)
            {
                if (left > TILE_CHUNK_WIDTH)
                    continue;
                else right = TILE_CHUNK_WIDTH;
            }
            if (top < 0)
            {
                if (bottom < 0)
                    continue;
                else top = 0;
            }
            if (bottom > TILE_CHUNK_HEIGHT)
            {
                if (top > TILE_CHUNK_HEIGHT)
                    continue;
                else bottom = TILE_CHUNK_HEIGHT;
            }

            //calculate tile indices for the tiles inside the box
            u32 upperLeft = left + (top * TILE_CHUNK_WIDTH);
            u32 upperRight  = right + (top * TILE_CHUNK_WIDTH);
            u32 lowerLeft = left + (bottom * TILE_CHUNK_WIDTH);

            u32 widthTiles = upperRight - (upperLeft - 1);

            //loop through all the tiles within the rectangle and try to find a hit
            for (u32 t1 = upperLeft; t1 != lowerLeft, result.hit == false; t1 += TILE_CHUNK_WIDTH)
            {
                for (u32 t2 = 0; t2 < widthTiles, result.hit == false; t2++)
                {
                    u32 chunkTileIndex = t1+t2;
                    u8 tilesetTileIndex = layer->chunk[overlappingChunks[i]].tile[chunkTileIndex];

                    const Tile tile = tileset->tiles[tilesetTileIndex];

                    if ((type != TILE_ANY && type != tile.type) || tile.type == TILE_EMPTY)
                        continue;

                    //hit found:

                    s32 xTile = (chunkTileIndex % TILE_CHUNK_WIDTH + chunkOffset.x) * TILE_SIZE;
                    s32 yTile = (chunkTileIndex / TILE_CHUNK_WIDTH + chunkOffset.y) * TILE_SIZE;

                    result.tile = tile;
                    result.pos = {xTile, yTile};
                    result.hit = true;
                }
            }

        }
    }

    return result;
}

TileHit *Collision::box_tile_collision_multiple(Rectangle2 rect, TileType type, int &count)
{
    TileLayer *layer = World::get_main_layer();

    TileHit *result = NULL;

    Rectangle2 boxInTileSpace = rect;
    boxInTileSpace.x1 /= TILE_SIZE;
    boxInTileSpace.x2 /= TILE_SIZE;
    boxInTileSpace.y1 /= TILE_SIZE;
    boxInTileSpace.y2 /= TILE_SIZE;

    //calculate how many tiles in box
    u32 boxWidthInTiles = std::ceil(boxInTileSpace.x2 - boxInTileSpace.x1);
    u32 boxHeightInTiles = std::ceil(boxInTileSpace.y2 - boxInTileSpace.y1);
    u32 tileCount = boxWidthInTiles * boxHeightInTiles;

    //reserve max amount of hits there can be
    TileHit hits[tileCount];
    for (int i = 0; i < tileCount; i++)
        hits[i].hit = false;
    u32 hitCount = 0;

    //find out how many chunks the box is intersecting with and store their indices
    u32 chunkCount = layer->chunkCount;
    u32 overlappingChunkCount = 0;
    u32 overlappingChunks[4];
    for (u32 i = 0; i < chunkCount, overlappingChunkCount < 4; i++)
    {
        fvec2 chunkOffset = layer->chunkOffset[i];
        Rectangle2 layerRect = {chunkOffset.x, chunkOffset.x + TILE_CHUNK_WIDTH, chunkOffset.y, chunkOffset.y + TILE_CHUNK_HEIGHT};

        if (rect_rect_collision(boxInTileSpace, layerRect))
        {
            overlappingChunks[overlappingChunkCount] = i;
            overlappingChunkCount++;
        }

    }

    if (overlappingChunkCount != 0)
    {
        Tileset *tileset = World::get_tileset();

        for (u32 i = 0; i < overlappingChunkCount; i++)
        {
            fvec2 chunkOffset = layer->chunkOffset[overlappingChunks[i]];

            //transform into chunk space and clip box to fit inside chunk
            s32 left = boxInTileSpace.x1 - chunkOffset.x;
            s32 right = boxInTileSpace.x2 - chunkOffset.x;
            s32 top = boxInTileSpace.y1 - chunkOffset.y;
            s32 bottom = boxInTileSpace.y2 - chunkOffset.y;

            if (left < 0)
            {
                if (right < 0)
                    continue;
                else left = 0;
            }
            if (right > TILE_CHUNK_WIDTH)
            {
                if (left > TILE_CHUNK_WIDTH)
                    continue;
                else right = TILE_CHUNK_WIDTH;
            }
            if (top < 0)
            {
                if (bottom < 0)
                    continue;
                else top = 0;
            }
            if (bottom > TILE_CHUNK_HEIGHT)
            {
                if (top > TILE_CHUNK_HEIGHT)
                    continue;
                else bottom = TILE_CHUNK_HEIGHT;
            }

            //calculate tile indices for the tiles inside the box
            u32 upperLeft = left + (top * TILE_CHUNK_WIDTH);
            u32 upperRight  = right + (top * TILE_CHUNK_WIDTH);
            u32 lowerLeft = left + (bottom * TILE_CHUNK_WIDTH);

            u32 widthTiles = upperRight - (upperLeft - 1);

            //loop through all the tiles within the rectangle and try to find a hit
            for (u32 t1 = upperLeft; t1 != lowerLeft; t1 += TILE_CHUNK_WIDTH)
            {
                for (u32 t2 = 0; t2 < widthTiles; t2++)
                {
                    u32 chunkTileIndex = t1+t2;
                    u8 tilesetTileIndex = layer->chunk[overlappingChunks[i]].tile[chunkTileIndex];

                    const Tile tile = tileset->tiles[tilesetTileIndex];

                    if ((type != TILE_ANY && type != tile.type) || tile.type == TILE_EMPTY)
                        continue;

                    //hit found:

                    s32 xTile = (chunkTileIndex % TILE_CHUNK_WIDTH + chunkOffset.x) * TILE_SIZE;
                    s32 yTile = (chunkTileIndex / TILE_CHUNK_WIDTH + chunkOffset.y) * TILE_SIZE;

                    hits[hitCount].tile = tile;
                    hits[hitCount].pos = {xTile, yTile};
                    hits[hitCount].hit = true;
                    hitCount++;

                    if(result == NULL)
                        result = hits;

                }
            }

        }
    }
    count = hitCount;
    return result;
}

/////////////////////////////////////////////
bool Collision::separating_axis_rects(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 ba, fvec2 bb, fvec2 bc, fvec2 bd)
{
    bool isSeparated = false;

    //v2 box1[4] = {aa, ab, ac, ad};
    //v2 box2[4] = {ba, bb, bc, bd};
    //projection axes
    fvec2 pa, pb, pc, pd, pe, pf, pg, ph;
    pa = ab - aa;
    pa = {-pa.y, pa.x};
    pb = ac - ab;
    pb = {-pb.y, pb.x};
    pc = ad - ac;
    pc = {-pc.y, pc.x};
    pd = aa - ad;
    pd = {-pd.y, pd.x};

    pe = bb - ba;
    pe = {-pe.y, pe.x};
    pf = bc - bb;
    pf = {-pf.y, pf.x};
    pg = bd - bc;
    pg = {-pg.y, pg.x};
    ph = ba - bd;
    ph = {-ph.y, ph.x};

    fvec2 projectionAxes[8] = {pa, pb, pc, pd, pe, pf, pg, ph};

    //loop thru projection axes
    for (int i = 0; i < 8; i++)
    {
        fvec2 axis = projectionAxes[i];

        //get min/max projections for boxes
        r32 aap = Inner(aa, axis);
        r32 abp = Inner(ab, axis);
        r32 acp = Inner(ac, axis);
        r32 adp = Inner(ad, axis);
        r32 aMin = aap;
        if (aMin > abp)
            aMin = abp;
        if (aMin > acp)
            aMin = acp;
        if (aMin > adp)
            aMin = adp;

        r32 aMax = aap;
        if (aMax < abp)
            aMax = abp;
        if (aMax < acp)
            aMax = acp;
        if (aMax < adp)
            aMax = adp;

        r32 bap = Inner(ba, axis);
        r32 bbp = Inner(bb, axis);
        r32 bcp = Inner(bc, axis);
        r32 bdp = Inner(bd, axis);
        r32 bMin = bap;
        if (bMin > bbp)
            bMin = bbp;
        if (bMin > bcp)
            bMin = bcp;
        if (bMin > bdp)
            bMin = bdp;

        r32 bMax = bap;
        if (bMax < bbp)
            bMax = bbp;
        if (bMax < bcp)
            bMax = bcp;
        if (bMax < bdp)
            bMax = bdp;

        bool result = (bMin > aMax || aMin > bMax);
        isSeparated = (isSeparated || result);
    }

    return !isSeparated;
}

bool Collision::separating_axis_rect_circle(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 cPos, u32 radius)
{
    bool isSeparated = false;

    //projection axes
    fvec2 pa, pb, pc, pd, pe, pf, pg, ph;
    pa = ab - aa;
    pa = {-pa.y, pa.x};
    pb = ac - ab;
    pb = {-pb.y, pb.x};
    pc = ad - ac;
    pc = {-pc.y, pc.x};
    pd = aa - ad;
    pd = {-pd.y, pd.x};

    fvec2 projectionAxes[4] = {pa, pb, pc, pd};

    //loop thru projection axes
    for (int i = 0; i < 4; i++)
    {
        fvec2 axis = projectionAxes[i];

        //get min/max projections for box
        r32 aap = Inner(aa, axis);
        r32 abp = Inner(ab, axis);
        r32 acp = Inner(ac, axis);
        r32 adp = Inner(ad, axis);
        r32 aMin = aap;
        if (aMin > abp)
            aMin = abp;
        if (aMin > acp)
            aMin = acp;
        if (aMin > adp)
            aMin = adp;

        r32 aMax = aap;
        if (aMax < abp)
            aMax = abp;
        if (aMax < acp)
            aMax = acp;
        if (aMax < adp)
            aMax = adp;

        //circle projection
        r32 cProj = Inner(cPos, axis);
        r32 bMin = cProj - radius;
        r32 bMax = cProj + radius;

        bool result = (bMin > aMax || aMin > bMax);
        isSeparated = (isSeparated || result);
    }
    return !isSeparated;
}

bool separating_axis_rect_point(fvec2 aa, fvec2 ab, fvec2 ac, fvec2 ad, fvec2 pPos)
{
    bool isSeparated = false;

    //projection axes
    fvec2 pa, pb, pc, pd, pe, pf, pg, ph;
    pa = ab - aa;
    pa = {-pa.y, pa.x};
    pb = ac - ab;
    pb = {-pb.y, pb.x};
    pc = ad - ac;
    pc = {-pc.y, pc.x};
    pd = aa - ad;
    pd = {-pd.y, pd.x};

    fvec2 projectionAxes[4] = {pa, pb, pc, pd};

    //loop thru projection axes
    for (int i = 0; i < 4; i++)
    {
        fvec2 axis = projectionAxes[i];

        //get min/max projections for box
        r32 aap = Inner(aa, axis);
        r32 abp = Inner(ab, axis);
        r32 acp = Inner(ac, axis);
        r32 adp = Inner(ad, axis);
        r32 aMin = aap;
        if (aMin > abp)
            aMin = abp;
        if (aMin > acp)
            aMin = acp;
        if (aMin > adp)
            aMin = adp;

        r32 aMax = aap;
        if (aMax < abp)
            aMax = abp;
        if (aMax < acp)
            aMax = acp;
        if (aMax < adp)
            aMax = adp;

        //circle projection
        r32 pProj = Inner(pPos, axis);

        bool result = (pProj > aMax || aMin > pProj);
        isSeparated = (isSeparated || result);
    }
    return !isSeparated;
}

bool rect_point_collision(Rectangle2 rect, fvec2 pPos)
{
    bool result;
    result = ((rect.x1 < pPos.x && pPos.x < rect.x2) && (rect.y1 < pPos.y && pPos.y < rect.y2));
    return result;
}

bool circle_point_collision(fvec2 cPos, u32 radius, fvec2 pPos)
{
    fvec2 distance = cPos - pPos;
    r32 length = Length(distance);

    bool result;
    result = (length <= radius);
    return result;
}

bool Collision::rect_rect_collision(Rectangle2 a, Rectangle2 b)
{
    bool result;
    result = ((a.x1 < b.x2 && b.x1 < a.x2) && (a.y1 < b.y2 && b.y1 < a.y2));
    return result;
}

/////////////////////////////////////////////

std::vector<fvec2> Collision::bresenham_line(fvec2 a, fvec2 b)
{
    std::vector<fvec2> result;

    bool steep = (absolute_value(b.y - a.y) > absolute_value(b.x - a.x));
    if (steep)
    {
        std::swap(a.x, a.y);
        std::swap(b.x, b.y);
    }
    if (a.x > b.x)
    {
        std::swap(a.x, b.x);
        std::swap(a.y, b.y);
    }

    s32 deltax = b.x - a.x;
    s32 deltay = absolute_value(b.y - a.y);
    s32 error = 0;
    s32 ystep;
    s32 y = a.y;
    if (a.y < b.y)
        ystep = 1;
    else ystep = -1;

    for (int x = a.x; x <= b.x; x++)
    {
        if (steep)
            result.push_back({y, x});
        else result.push_back({x, y});

        error += deltay;
        if (2 * error >= deltax)
        {
            y += ystep;
            error -= deltax;
        }
    }

    return result;
}

bool Collision::tile_point_free(Tile tile, fvec2 relativePos)
{
    if (relativePos.x >= TILE_SIZE || relativePos.y >= TILE_SIZE)
        throw std::runtime_error("tile collision weirdness");

    u32 height = tile.height[relativePos.x];
    bool result;

    switch (tile.type)
    {
    case TILE_EMPTY:
        result = true;
        break;
    case TILE_SOLID:
    case TILE_LEDGE:
        result = false;
        break;
    case TILE_PASS_THROUGH:
    case TILE_JUMP_THROUGH:
        if (relativePos.y < (TILE_SIZE - height))
            result = true;
        else result = false;
        break;
    case TILE_PASS_THROUGH_FLIP:
        if (relativePos.y < height)
            result = false;
        else result = true;
        break;
    default:
        result = true;
        break;
    }

    return result;
}

bool Collision::collision_tile_mask(fvec2 positionInPx)
{
    bool result = false;

    TileLayer *layer = World::get_main_layer();
    Tileset *tileset = World::get_tileset();

    u32 chunkCount = layer->chunkCount;
    for (u32 i = 0; i < chunkCount; i++)
    {
        ivec2 chunkOffset = layer->chunkOffset[i];
        //position in chunk space
        f32 x = positionInPx.x - chunkOffset.x;
        f32 y = positionInPx.y - chunkOffset.y;
        u32 floor_x = std::floor((r32)x);
        u32 floor_y = std::floor((r32)y);
        f32 mod_x = x - floor_x;
        f32 mod_y = y - floor_y;
        //offsets to tile
        u32 tileSpace_x = mod_x * TILE_SIZE;
        u32 tileSpace_y = mod_y * TILE_SIZE;

        u32 tileIndex = (TILE_CHUNK_WIDTH * floor_y) + floor_x;
        if (tileIndex >= TILE_CHUNK_WIDTH * TILE_CHUNK_HEIGHT)
            continue;

        u32 indexToTileset = layer->chunk[i].tile[tileIndex];

        const Tile tile = tileset->tiles[indexToTileset];
        bool tilePointFree = tile_point_free(tile, {tileSpace_x, tileSpace_y});
        if (!tilePointFree)
        {
            result = true;
            break;
        }
    }

    return result;
}

RaycastHit Collision::cast_ray(fvec2 position, v2 direction, r32 lengthInPx)
{
    RaycastHit result = {};
    fvec2 positionInPx;
    positionInPx.x = std::round((r32)position.x);
    positionInPx.y = std::round((r32)position.y);

    if (lengthInPx == 0)
    {
        result.hit = collision_tile_mask(positionInPx);
        result.length = 0;
        result.pos = positionInPx;
    }
    else
    {
        std::vector<fvec2> rayLine = bresenham_line(positionInPx, positionInPx + (Normalize(direction) * lengthInPx));

        if (rayLine.size() > 0)
        {
            s32 rayPointIndex = 0;

            if (rayLine[0] != positionInPx)
                rayPointIndex = rayLine.size() - 1;

            //loop through all points on the ray
            while (true)
            {
                fvec2 rayPoint = rayLine[rayPointIndex];
                if (collision_tile_mask(rayPoint))
                {
                    result.hit = true;
                    result.length = Length(positionInPx - rayPoint);
                    result.pos = rayPoint;
                    break;
                }

                if (rayLine[0] != positionInPx)
                {
                    rayPointIndex--;
                    if (rayPointIndex < 0)
                        break;
                }
                else
                {
                    rayPointIndex++;
                    if (rayPointIndex >= rayLine.size())
                        break;
                }
            }
        }
    }

    return result;
}
