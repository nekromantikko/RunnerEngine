#ifndef TEXTURE_H
#define TEXTURE_H

#include "../shared.h"

struct InternalTexture;

enum TextureType
{
    TEXTURE_BASE_COLOR,
    TEXTURE_PALETTE,
    TEXTURE_COLOR_NDX,
    TEXTURE_NORMAL,
    TEXTURE_TILE_NDX,
};

struct Texture
{
    InternalTexture *internal;
    u32 width, height;
};

#endif // TEXTURE_H
