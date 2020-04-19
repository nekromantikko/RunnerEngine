#include "tiles.h"
#include <fstream>
#include "file_formats.h"
#include "resource.h"
#include "renderer.h"
#include "Platform/platform.h"
#include "debug.h"

//tileset functions
Tileset::Tileset(std::string fname) : texture_(NULL), lightmap_(NULL), normal_(NULL)
{
    load(fname);
    create_texcoords();
    std::cout << "Tileset <" << this << "> created!\n";
}

Tileset::~Tileset()
{
    std::cout << "Tileset <" << this << "> destroyed!\n";
}

Texture *Tileset::get_diffuse()
{
    return texture_;
}

Texture *Tileset::get_lightmap()
{
    return lightmap_;
}

Texture *Tileset::get_normal()
{
    return normal_;
}

memory_index Tileset::get_size() const
{
    return tiles.size();
}

Tile *Tileset::get_tile(memory_index index)
{
    return &tiles.at(index);
}

v2 Tileset::get_coord(memory_index index)
{
    if (index >= texCoords.size())
    {
        if (texCoords.empty())
            return {0,0};
        else return texCoords.back();
    }
    else return texCoords.at(index);
}

void Tileset::load(std::string fname)
{
    std::ifstream file(fname, std::ios::binary);

    if (file)
    {
        TilesetHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != TILESET_SIGNATURE)
            throw std::runtime_error("Tileset is not a proper rtil-file!");
        else if (header.version != TILESET_VERSION)
            throw std::runtime_error("Tileset file is not the right version!");
        else
        {
            //read texture filename
            texture_ = NULL;
            u32 textureFilenameLength = header.lightmapOffset - header.textureOffset;
            if (textureFilenameLength)
            {
                char *textureFilenameBuffer = new char[textureFilenameLength];
                file.read(textureFilenameBuffer, textureFilenameLength);
                std::string textureFilename(textureFilenameBuffer, textureFilenameLength);
                texture_ = Resource::get_texture(textureFilename);
                delete[] textureFilenameBuffer;
            }
            if (!texture_)
                texture_ = Resource::no_texture();

            //read lightmap filename
            lightmap_ = NULL;
            u32 lightmapFilenameLength = header.normalOffset - header.lightmapOffset;
            if (lightmapFilenameLength)
            {
                char *lightmapFilenameBuffer = new char[lightmapFilenameLength];
                file.read(lightmapFilenameBuffer, lightmapFilenameLength);
                std::string lightmapFilename(lightmapFilenameBuffer, lightmapFilenameLength);
                lightmap_ = Resource::get_texture(lightmapFilename);
                delete[] lightmapFilenameBuffer;
            }
            if (!lightmap_)
                lightmap_ = Resource::no_lightmap();


            //read normal filename
            normal_ = NULL;
            u32 normalFilenameLength = header.tilesOffset - header.normalOffset;
            if (normalFilenameLength)
            {
                char *normalFilenameBuffer = new char[normalFilenameLength];
                file.read(normalFilenameBuffer, normalFilenameLength);
                std::string normalFilename(normalFilenameBuffer, normalFilenameLength);
                normal_ = Resource::get_texture(normalFilename);
                delete[] normalFilenameBuffer;
            }
            if (!normal_)
                normal_ = Resource::no_normal();


            //read tiles
            for (memory_index i = 0; i < header.tileCount; i++)
            {
                Tile tile;
                file.read((char*)&tile, sizeof(TileInfo));

                //replace framecount with index
                u32 frameCount = tile.index;
                tile.index = i;

                tiles.push_back(tile);

                if (frameCount)
                {
                    TileAnim anim;
                    anim.accumulator = 0.0;
                    anim.currentFrame = 0;
                    for (memory_index j = 0; j < frameCount; j++)
                    {
                        TileAnimFrame frame;
                        file.read((char*)&frame, sizeof(TileAnimFrame));
                        anim.frames.push_back(frame);
                    }
                    anim.tileIndex = &tiles.back().index;
                    animations.push_back(anim);
                }

            }

        }
    }
    else throw std::runtime_error("can't open tileset file!");

    file.close();
}

void Tileset::create_texcoords()
{
    //clipping
    r32 texLeft, texTop;

    if (texture_)
    {
        //texture width / height in frames
        u32 texWidth;
        platform_get_texture_width(texture_, &texWidth);
        u32 widthInFrames = texWidth / runnerTileSize;
        u32 texHeight;
        platform_get_texture_height(texture_, &texHeight);
        u32 heightInFrames = texHeight / runnerTileSize;

        u32 frameCount = widthInFrames * heightInFrames;


        if (frameCount != 0)
        {
            texCoords.reserve(frameCount);

            for (memory_index i = 0; i < frameCount; i++)
            {
                texLeft = (i % widthInFrames) / (r32)widthInFrames;
                texTop = (i / widthInFrames) / (r32)heightInFrames;

                v2 coord = {texLeft, texTop};
                texCoords.push_back(coord);
            }
        }
    }
}

void Tileset::animate()
{
    for (auto &anim : animations)
    {
        anim.accumulator += 1000.f / (anim.frames.at(anim.currentFrame).duration * 60);

        while (anim.accumulator >= 1)
        {
            anim.accumulator -= 1;
            anim.currentFrame++;
            if (anim.currentFrame >= anim.frames.size())
                anim.currentFrame = 0;
            u32 &tile = *anim.tileIndex;
            tile = anim.frames.at(anim.currentFrame).frame;
        }
    }
}

namespace TileManager
{
    #define MAX_TILELAYER_AMOUNT 64
    u32 layerAmount = 0;
    TileLayer layers[MAX_TILELAYER_AMOUNT];
}

//TILEMANAGER FUNCTIONS

void TileManager::create_layer(std::vector<TilesetAndGid> tilesets, TileLayerInfo &info, u32 *tiles)
{
    TileLayer &layer = layers[layerAmount++];

    layer.width = info.width;
    layer.height = info.height;
    layer.z = info.z;
    layer.xSpeed = info.xSpeed;
    layer.ySpeed = info.ySpeed;
    layer.xTiling = info.xTiling;
    layer.yTiling = info.yTiling;
    layer.collision = info.collision;
    layer.tileAmount = info.tileAmount;

    for (auto it = tilesets.rbegin(); it != tilesets.rend(); it++)
    {
        TileLayerData temp;
        temp.tileset = it->tileset;

        for (memory_index i = 0; i < layer.tileAmount; i++)
        {
            u32 tile = tiles[i];
            if (tile < it->gid || tile >= it->tileset->get_size() + it->gid)
            {
                temp.tiles.push_back(NULL);
                continue;
            }

            u32 actualGid = tile - it->gid;
            temp.tiles.push_back(&it->tileset->get_tile(actualGid)->index);
        }

        layer.data.push_back(temp);
    }
}

void TileManager::create_layer_textures()
{
    //loop thry layers
    for (int i = 0; i < layerAmount; i++)
    {
        TileLayer &layer = layers[i];
        for (auto it = layer.data.rbegin(); it != layer.data.rend(); it++)
        {
            //try a thing
            std::vector<v3> texels;
            for (u32 *tile : it->tiles)
            {
                v3 texel = {0,0,0};

                if (tile)
                {
                    texel.xy() = it->tileset->get_coord(*tile);
                    texel.z = 1.0;
                }

                texels.push_back(texel);
            }

            it->layout = platform_create_tile_layer_texture(texels.data(), layer.width, layer.height);
        }
    }
}

//draws tiles if the layer is not invisible (disabled)
void TileManager::update()
{
    //Resource::animate_tilesets();

    for (u32 i = 0; i < layerAmount; i++)
    {
        TileLayer *layer = &layers[i];
        //update_textures(layer);
        draw_layer(layer);
    }
}

void TileManager::update_textures(TileLayer *layer)
{
    u32 tilesetAmount = layer->data.size();

    for (u32 i = 0; i < tilesetAmount; i++)
    {
        Tileset *tileset = layer->data[i].tileset;
        std::vector<v3> texels;
        for (u32 *tile : layer->data[i].tiles)
        {
            v3 texel = {0,0,0};

            if (tile)
            {
                texel.xy() = tileset->get_coord(*tile);
                texel.z = 1.0;
            }

            texels.push_back(texel);
        }
        platform_update_tile_layer_texture(layer->data[i].layout, texels.data(), layer->width, layer->height);
    }
}

void TileManager::draw_layer(TileLayer *layer)
{
    if (layer->width == 0 || layer->height == 0)
        return;

    //the program takes the first camera entity if one exists
    //Camera *camera = static_cast<Camera*>(EntitySystem::get_entity(Camera::type()));
    //get camera position. If there's no camera, it's just zeroes
    fvec2 camPos = Renderer::get_camera_position();
    f32 xCamera = camPos.x, yCamera = camPos.y;

    //get layer size in pixels
    s32 tileSizeInPx = runnerTileSize;
    u32 widthInPx = layer->width * tileSizeInPx;
    u32 heightInPx = layer->height * tileSizeInPx;
    //amount of times the layer should be repeated on a given axis (default is 1)
    //u32 xRepetitions = 1, yRepetitions = 1;
    //the layer's position relative to the camera and layer's parallax scrolling rate
    r32 xScrollDistance = xCamera * layer->xSpeed;
    r32 yScrollDistance = yCamera * layer->ySpeed;
    //where should the drawing begin (the first tiles might be  partially outside the drawing area)
    r32 xStart = -xScrollDistance;
    r32 yStart = -yScrollDistance;

    //even I can no longer remember what this shit means
    if (layer->xTiling)
    {
        xStart = -xScrollDistance - ceil(-xScrollDistance / widthInPx) * widthInPx;
        //xRepetitions = (u32)ceil((r32)(runnerScreenWidth - xStart) / widthInPx);
    }
    //just believe me that it works
    if (layer->yTiling)
    {
        yStart = -yScrollDistance - ceil(-yScrollDistance / heightInPx) * heightInPx;
        //yRepetitions = (u32)ceil((r32)(runnerScreenHeight - yStart) / heightInPx);
    }

    Transform xform;
    xform.position = {std::round(xStart), std::round(yStart)};
    Transform &previous = layer->previous;

    //fix parallax for lerping:

    u32 xDifference = absolute_value(xform.position.x - previous.position.x);
    if (xDifference >= runnerTileSize)
    {
        if (xform.position.x < previous.position.x)
        {
            previous.position.x -= widthInPx;
        }
        else if (xform.position.x > previous.position.x)
        {
            previous.position.x += widthInPx;
        }
    }

    u32 yDifference = absolute_value(xform.position.y - previous.position.y);
    if (yDifference >= runnerTileSize)
    {
        if (xform.position.y < previous.position.y)
        {
            previous.position.y -= heightInPx;
        }
        else if (xform.position.y > previous.position.y)
        {
            previous.position.y += heightInPx;
        }
    }

    u32 tilesetAmount = layer->data.size();

    for (u32 i = 0; i < tilesetAmount; i++)
    {
        TileCall call;
        call.z = layer->z;
        call.texture = layer->data[i].tileset->get_diffuse();
        call.lightmap = layer->data[i].tileset->get_lightmap();
        call.normal = layer->data[i].tileset->get_normal();
        call.a = xform.position;
        call.b = previous.position;
        call.layout = layer->data[i].layout;
        Renderer::add_tile_layer(call);
    }

    previous = xform;
}

void TileManager::clear_layers()
{
    for (memory_index i = 0; i < layerAmount; i++)
    {
        //for (TileLayerData &data : layers[i].data)
            //platform_delete_vertex_buffer(data.vbuffer);

        TileLayer *layer = &layers[i];

        u32 tilesetAmount = layer->data.size();
        for (u32 i = 0; i < tilesetAmount; i++)
            platform_delete_texture(layer->data[i].layout);

        layer->data.clear();
    }

    layerAmount = 0;
}

TileLayer *TileManager::get_layers()
{
    return layers;
}

u32 TileManager::get_layer_amount()
{
    return layerAmount;
}

s32 TileManager::get_tile_height(Tile *tile, s32 x)
{
    return tile->mask[x];
}
