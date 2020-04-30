#include "world.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "Platform/platform.h"
#include <cstring>
#include "resource.h"

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

Tileset *World::get_tileset()
{
    return &tileset;
}

void World::create_index_maps()
{
    mainLayer.chunkTextureCount = mainLayer.chunkCount / CHUNKS_PER_TEXTURE + 1;

    std::cout << "Creating " << mainLayer.chunkTextureCount << " tile chunk textures\n";

    u32 chunkSize = TILE_CHUNK_WIDTH * TILE_CHUNK_HEIGHT;
    mainLayer.chunkTexture = new Texture*[mainLayer.chunkTextureCount];

    u32 textureSize = CHUNKS_PER_TEXTURE * chunkSize;
    u8 pixels[textureSize * mainLayer.chunkTextureCount];

    int chunksRemaining = mainLayer.chunkCount;
    for (int i = 0; i < mainLayer.chunkTextureCount; i++)
    {
        std::cout << "Creating chunk texture with " << std::min(chunksRemaining, CHUNKS_PER_TEXTURE) << " chunks\n";
        mainLayer.chunkTexture[i] = platform_create_tile_index_map(chunkSize, CHUNKS_PER_TEXTURE);
        for (u32 chunkIndex = 0; chunkIndex < CHUNKS_PER_TEXTURE * (i+1); chunkIndex++)
        {
            if (chunksRemaining > 0)
                memcpy(pixels + i * textureSize + chunkIndex * chunkSize, &mainLayer.chunk[chunkIndex], chunkSize);
            chunksRemaining--;
        }
        platform_populate_tile_index_map(mainLayer.chunkTexture[i], chunkSize, CHUNKS_PER_TEXTURE, pixels + i * textureSize);
    }
}

void World::clear_index_maps()
{
    for (int i = 0; i < mainLayer.chunkTextureCount; i++)
        platform_delete_texture(mainLayer.chunkTexture[i]);
    delete[] mainLayer.chunkTexture;
}

void World::update()
{

}

/////////////////////////////////////////////////////////
bool World::load_tileset(const char *fname)
{
    std::cout << "Trying to load tileset " << fname << std::endl;
    bool result = false;

    rapidxml::file<> xmlFile(fname);
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    rapidxml::xml_node<> *tilesetNode = doc.first_node("tileset");

    char *version;
    version = tilesetNode->first_attribute("version")->value();

    //check for correct version (to reduce headaches later if the format changes)
    if (strcmp(version, "1.2") == 0)
    {
        char *name;
        name = tilesetNode->first_attribute("name")->value();

        char ndx[128], normal[128];
        strcat(ndx, name);
        strcat(ndx, "_ndx");
        strcat(normal, name);
        strcat(normal, "_normal");

        tileset.baseColor = Resource::Loader::load_texture(ndx, false);
        tileset.normalMap = Resource::Loader::load_texture(normal, false);

        u32 tilesetSideLength = TILE_SIZE * 16;

        //generate height masks
        char maskFname[128];
        strcat(maskFname, "res/textures/tilesets/");
        strcat(maskFname, name);
        strcat(maskFname, "/");
        strcat(maskFname, name);
        strcat(maskFname, "_mask.png");
        std::cout << "Trying to load tileset mask " << maskFname << std::endl;
        rImage maskImage = platform_load_image(maskFname);
        std::cout << "Mask dimensions: " << maskImage.width << ", " << maskImage.height <<std::endl;

        if (maskImage.width == tilesetSideLength && maskImage.height == tilesetSideLength)
        {
            for(int i = 0; i < 256; i++)
            {
                //check alpha
                if (maskImage.pixels[(4 * i) + 3] == 0)
                    continue;

                s32 xTile = (i % tilesetSideLength) / TILE_SIZE;
                s32 yTile = (i / tilesetSideLength) / TILE_SIZE;
                s32 x = (i % tilesetSideLength) % TILE_SIZE;

                s32 tilesPerRow = tilesetSideLength / TILE_SIZE;
                s32 tile = yTile * tilesPerRow + xTile;

                tileset.tiles[tile].height[x] += 1;
            }
            platform_delete_image(maskImage);

            //load tile data into memory
            int counter = 0;
            for (rapidxml::xml_node<> *xmlTile = tilesetNode->first_node("tile"); counter < 256; xmlTile = xmlTile->next_sibling("tile"), counter++)
            {
                std::cout << counter << ", " << xmlTile << std::endl;

                //check if tile has any data in the file
                if (xmlTile != NULL)
                {
                    u8 tileIndex = atoi(xmlTile->first_attribute("id")->value());

                    //not all tiles are in the tileset, so put those in as empty tiles
                    while (tileIndex > counter)
                    {
                        tileset.tiles[counter].slope = 0.0;
                        tileset.tiles[counter].type = TILE_EMPTY;
                        counter++;
                    }

                    char *type;
                    type = xmlTile->first_attribute("type")->value();

                    if (strcmp(type, "TILE_ANY") == 0
                        || strcmp(type, "ANY") == 0
                        || strcmp(type, "TILE_EMPTY") == 0
                        || strcmp(type, "EMPTY") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_EMPTY;
                    }
                    else if (strcmp(type, "TILE_SOLID") == 0
                        || strcmp(type, "SOLID") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_SOLID;
                    }
                    else if (strcmp(type, "TILE_PASS_THROUGH") == 0
                        || strcmp(type, "PASS_THROUGH") == 0
                        || strcmp(type, "TILE_PASS_THRU") == 0
                        || strcmp(type, "PASS_THRU") == 0
                        || strcmp(type, "TILE_SLOPE") == 0
                        || strcmp(type, "SLOPE") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_SLOPE;
                    }
                    else if (strcmp(type, "TILE_JUMP_THROUGH") == 0
                        || strcmp(type, "JUMP_THROUGH") == 0
                        || strcmp(type, "TILE_JUMP_THRU") == 0
                        || strcmp(type, "JUMP_THRU") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_JUMP_THRU;
                    }
                    else if (strcmp(type, "TILE_PASS_THROUGH_FLIP") == 0
                        || strcmp(type, "PASS_THROUGH_FLIP") == 0
                        || strcmp(type, "TILE_PASS_THRU_FLIP") == 0
                        || strcmp(type, "PASS_THRU_FLIP") == 0
                        || strcmp(type, "TILE_SLOPE_FLIP") == 0
                        || strcmp(type, "SLOPE_FLIP") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_SLOPE_FLIP;
                    }
                    else if (strcmp(type, "TILE_LEDGE") == 0
                        || strcmp(type, "LEDGE") == 0)
                    {
                        tileset.tiles[tileIndex].type = TILE_LEDGE;
                    }

                    //loop thru properties
                    rapidxml::xml_node<> *properties = xmlTile->first_node("properties");
                    if (properties)
                    {
                        for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
                        {
                            char *pname = property->first_attribute("name")->value();

                            //figure out which attribute this is
                            if (strcmp(pname, "slope") == 0)
                            {
                                tileset.tiles[tileIndex].slope = atof(property->first_attribute("value")->value());
                                continue;
                            }
                        }
                    }
                }
                //fill rest of tileset with empty tiles
                else
                {
                    while (counter < 256)
                    {
                        tileset.tiles[counter].slope = 0.0;
                        tileset.tiles[counter].type = TILE_EMPTY;
                        counter++;
                    }
                    break;
                }


            }
            std::cout << "Tileset " << fname << " loaded successfully!\n";
        }
        else
        {
            std::cout << "Tileset " << fname << " mask image is not the correct size!\nShould be "
            << tilesetSideLength << " x " << tilesetSideLength << std::endl;
            platform_delete_image(maskImage);
        }

    }
    else
    {
        std::cout << "Tileset " << fname << " is not the correct version\n";
    }

    return result;
}

void World::unload_tileset()
{
    platform_delete_texture(tileset.baseColor);
    platform_delete_texture(tileset.normalMap);
}

bool World::load_map(const char *fname)
{
    std::cout << "Trying to load map " << fname << std::endl;
    bool result = false;

    rapidxml::file<> xmlFile(fname);
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    rapidxml::xml_node<> *mapNode = doc.first_node("map");

    char *version;
    version = mapNode->first_attribute("version")->value();

    //check for correct version (to reduce headaches later if the format changes)
    if (strcmp(version, "1.2") == 0)
    {
        rapidxml::xml_node<> *tilesetNode = mapNode->first_node("tileset");

        char *tilesetFirstGid;
        tilesetFirstGid = tilesetNode->first_attribute("firstgid")->value();
        int firstGid = atoi(tilesetFirstGid);

        char *tilesetSource;
        tilesetSource = tilesetNode->first_attribute("source")->value();
        char tilesetFname[256];
        strcat(tilesetFname, "res");
        strcat(tilesetFname, tilesetSource+2);
        load_tileset(tilesetFname);

        for (rapidxml::xml_node<> *layerNode = mapNode->first_node("layer"); layerNode; layerNode = layerNode->next_sibling("layer"))
        {
            char *layerName = layerNode->first_attribute("name")->value();

            //load only main layer for now
            if (strcmp(layerName, "MAIN") != 0)
                continue;

            rapidxml::xml_node<> *layerData = layerNode->first_node("data");

            //loop through chunks once first to count them
            mainLayer.chunkCount = 0;
            for (rapidxml::xml_node<> *chunkNode = layerData->first_node("chunk"); chunkNode; chunkNode = chunkNode->next_sibling("chunk"))
            {
                mainLayer.chunkCount++;
            }
            mainLayer.chunk = new TileChunk[mainLayer.chunkCount];
            mainLayer.chunkOffset = new ivec2[mainLayer.chunkCount];

            //loop thru them again to load the data
            u32 chunkIndex = 0;
            for (rapidxml::xml_node<> *chunkNode = layerData->first_node("chunk"); chunkNode; chunkNode = chunkNode->next_sibling("chunk"))
            {
                char *x, *y;
                x = chunkNode->first_attribute("x")->value();
                y = chunkNode->first_attribute("x")->value();

                mainLayer.chunkOffset[chunkIndex] = ivec2(atoi(x), atoi(y));

                u32 tileIndex = 0;
                for (rapidxml::xml_node<> *tile = chunkNode->first_node("tile"); tile; tile = tile->next_sibling("tile"), tileIndex++)
                {
                    u8 t;
                    rapidxml::xml_attribute<> *gid = tile->first_attribute("gid");
                    if (gid)
                    {
                        t = atoi(gid->value()) - firstGid;
                    }
                    else t = 0;

                    mainLayer.chunk[chunkIndex].tile[tileIndex] = t;
                }
            }

            mainLayer.scroll = {1.0f,1.0f};
            mainLayer.z = 0;
            mainLayer.xTiling = false;
            mainLayer.yTiling = false;

            result = true;
        }

    }
    else
    {
        std::cout << "Map " << fname << " is not the correct version\n";
    }

    return result;
}

void World::unload_map()
{
    clear_index_maps();
    mainLayer.chunkTexture = NULL;

    delete[] mainLayer.chunk;
    mainLayer.chunk = NULL;

    delete[] mainLayer.chunkOffset;
    mainLayer.chunkOffset = NULL;
}
