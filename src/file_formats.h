#ifndef FILE_FORMATS_H
#define FILE_FORMATS_H
#include "shared.h"

enum AssetType
{
    TEXTURE,
    SOUND,
    MUSIC,
    SPRITE,
    MODEL,
    MESH,
    TILESET,
    LEVEL,
    SHADER
};

#define SIGNATURE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

struct PakHeader
{
#define PAK_SIGNATURE SIGNATURE('r','p','a','k')
    u32 signature;
#define PAK_VERSION 0
    u32 version;

    u32 assetCount;

    u64 assetOffset; //PakAsset[assetCount]
};

struct PakAsset
{
    u32 type;
    //NOTE: the offsets should be added to the asset's offset!!
    u64 nameOffset;
    u64 dataOffset;
    u64 onePastDataOffset;
};

//////////////////////////////////////////////////////////////

struct TilesetHeader
{
    #define TILESET_SIGNATURE SIGNATURE('r','t','i','l')
    u32 signature;
    #define TILESET_VERSION 0
    u32 version;

    u32 tileCount;

    //the first offset is zero
    u64 textureOffset;
    u64 lightmapOffset;
    u64 normalOffset;
    u64 tilesOffset;
};

struct TileInfo
{
    u32 type;
    r32 slope;
    u32 mask[runnerTileSize];
    u32 frameCount;
};

struct TileAnimFrameInfo
{
    u32 frame;
    u32 duration;
};

///////////////////////////////////////////////////////////////

struct LevelHeader
{
    #define LEVEL_SIGNATURE SIGNATURE('r','l','v','l')
    u32 signature;
    #define LEVEL_VERSION 3
    u32 version;

    //level properties
    u32 width;
    u32 height;
    r32 ambientColor[4];
    r32 bgColor1[4];
    r32 bgColor2[4];

    u32 tilesetAmount;
    u32 tileLayerAmount;
    u32 entityAmount;

    u64 nameOffset;
    //position of first TilesetInfo in the file
    u64 tilesetOffset; // = sizeof(LevelHeader)
    //position of first TileLayerInfo
    u64 tileLayerOffset;
    //etc.
    u64 entityOffset;
    u64 onePastEntityOffset;
};

struct TilesetInfo
{
    u64 filenameOffset;
    u64 onePastFilenameOffset;
};

struct TileBSPNodeInfo
{
    Rectangle2 bounds;
    u32 tileCount;
    u32 offset;
};

struct TileBSPTreeInfo
{
    u32 nodeCount;
    u32 fullTreeSize;
};

struct TileLayerInfo
{
    u32 width;
    u32 height;
    r32 xScroll;
    r32 yScroll;
    bool32 xTiling;
    bool32 yTiling;
    u32 z;
    bool32 collision;
};

struct EntityInfo
{
    u32 x;
    u32 y;
    u32 w;
    u32 h;
    r32 depth;

    u32 propertyAmount;

    u64 nameOffset;
    u64 typeOffset;
    u64 propertyOffset;
    u64 onePastPropertyOffset;
};

struct EntityPropertyInfo
{
    u64 nameOffset;
    u64 valueOffset;
    u64 onePastValueOffset;
};

///////////////////////////////////////////////////////////

struct SpriteHeader
{
    #define SPRITE_SIGNATURE SIGNATURE('r','s','p','r')
    u32 signature;
    #define SPRITE_VERSION 0
    u32 version;

    u32 spriteCount;

    u64 spritesOffset;
};

struct SpriteInfo
{
    u32 width;
    u32 height;
    r32 xoffset;
    r32 yoffset;
    r32 glow;

    u32 animCount;

    u64 nameOffset;
    u64 textureOffset;
    u64 lightmapOffset;
    u64 normalOffset;
    u64 animsOffset;
    u64 onePastAnimsOffset;
};

struct SpriteAnimationInfo
{
    u32 frameCount;

    u64 nameOffset;
    u64 framesOffset;
    u64 onePastFramesOffset;
};

struct SpriteFrameInfo
{
    u32 index;
};

//////////////////////////////////////////////

struct ParticleHeader
{
    #define PARTICLE_SIGNATURE SIGNATURE('r','p','a','r')
    u32 signature;
    #define PARTICLE_VERSION 1
    u32 version;

    u32 systemCount;
    u64 systemOffset;
};

struct ParticleSystemInfo
{
    u32 emitterCount;

    u64 nameOffset;
    u64 emitterOffset;
    u64 onePastEmitterOffset;
};

struct ParticleEmitterInfo
{
    u32 animation;
    r32 spawnRate;
    r32 duration;
    u32 loops;
    u32 gravity;
    u32 spawnAreaType;
    u32 visualType;
    u8 spawnAreaData[8];

    r32 lifeTime[2];
    r32 initialScale[2];
    r32 initialVelocity[4];
    r32 initialRotation[2];
    r32 animSpeed[2];

    u64 spriteOffset;
    u64 onePastSpriteOffset;
};

//////////////////////////////////////////

struct ModelHeader
{
    #define MODEL_SIGNATURE SIGNATURE('r','m','d','l')
    u32 signature;
    #define MODEL_VERSION 0
    u32 version;

    u32 modelCount;
    u64 modelsOffset;
};

struct ModelInfo
{
    r32 glow;

    u64 nameOffset;
    u64 textureOffset;
    u64 lightmapOffset;
    u64 normalOffset;
    u64 vertexOffset;
    u64 indexOffset;
    u64 onePastIndexOffset;
};

struct ModelVertInfo
{
    r32 x, y, z;
    r32 u, v;
    r32 nx, ny, nz;

    bool operator==(const ModelVertInfo& other)
    {
        return (x == other.x && y == other.y && z == other.z && u == other.u && v == other.v && nx == other.nx && ny == other.ny && nz == other.nz);
    }
};

////////////////////////////////////////

struct MultiSoundHeader
{
    #define MULTISOUND_SIGNATURE SIGNATURE('r','s','n','d')
    u32 signature;
    #define MULTISOUND_VERSION 0
    u32 version;

    u32 multiSoundCount;
    u64 multiSoundsOffset;
};

struct MultiSoundInfo
{
    u32 soundCount;

    u64 nameOffset;
    u64 soundsOffset;
    u64 onePastSoundsOffset;
};

struct MultiSoundSoundInfo
{
    u64 nameOffset;
    u64 onePastNameOffset;
};

///////////////////////////////////////////

struct InputMacroHeader
{
    #define MACRO_SIGNATURE SIGNATURE('r','m','a','c')
    u32 signature;
    #define MACRO_VERSION 0
    u32 version;

    u32 frameCount;
};

#endif // FILE_FORMATS_H
