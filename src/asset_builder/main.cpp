#include <fstream>
#include <string>
#include <sstream>
#include "../file_formats.h"
#include <vector>
#include <iostream>
#include "../compression.h"
#include "../rapidxml/rapidxml.hpp"
#include "../rapidxml/rapidxml_utils.hpp"
#include <utility>
#include <IL/il.h>
#include <IL/ilu.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../bsp.h"

rImage load_image(const char* fname)
{
    //generate and set current image ID
    ILuint imgID;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    //load
    ilLoadImage(fname);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    rImage temp;
    temp.width = ilGetInteger(IL_IMAGE_WIDTH);
    temp.height = ilGetInteger(IL_IMAGE_HEIGHT);

    s32 imageSize = temp.width * temp.height * 4;
    temp.pixels = new u8[imageSize];

    memcpy(temp.pixels, ilGetData(), imageSize);

    //delete file from memory
    ilDeleteImages(1, &imgID);

    return temp;
}

void delete_image(rImage image)
{
    delete[] image.pixels;
}

struct AssetData
{
    AssetType type;
    std::string name;
    std::string data;
};

std::string load_file_data(const char* fname)
{
    std::ifstream file(fname, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#define PUSH_ASSET(asset_type, asset_name, asset_fname, vector) \
{ AssetData asset = {};\
asset.type = asset_type;\
asset.name = asset_name;\
asset.data = load_file_data(asset_fname);\
vector.push_back(asset); }

void build_assets()
{
    std::cout << "PACKING ASSETS...\n";
    //load all the assets
    std::vector<AssetData> assets;

    //load models
    PUSH_ASSET(MODEL, "mdl_medkit", "res/models/mdl_medkit.mdl", assets);
    PUSH_ASSET(MODEL, "mdl_spikething", "res/models/mdl_spikething.mdl", assets);
    PUSH_ASSET(MODEL, "mdl_wall01", "res/models/mdl_wall01.mdl", assets);
    PUSH_ASSET(MODEL, "mdl_wall02", "res/models/mdl_wall02.mdl", assets);
    PUSH_ASSET(MODEL, "mdl_wall03", "res/models/mdl_wall03.mdl", assets);

    //load objs
    PUSH_ASSET(MESH, "medkit", "res/models/medkit.obj", assets);
    PUSH_ASSET(MESH, "spikething", "res/models/pikething.obj", assets);
    PUSH_ASSET(MESH, "wall01", "res/models/wall01.obj", assets);
    PUSH_ASSET(MESH, "wall02", "res/models/wall02.obj", assets);
    PUSH_ASSET(MESH, "wall03", "res/models/wall03.obj", assets);

    //load music
    PUSH_ASSET(MUSIC, "music_test", "res/music/music_test.mod", assets);

    //load shaders
    PUSH_ASSET(SHADER, "background_shader.glfs", "res/shaders/background_shader.glfs", assets);
    PUSH_ASSET(SHADER, "background_shader.glvs", "res/shaders/background_shader.glvs", assets);
    PUSH_ASSET(SHADER, "deferred_shader.glfs", "res/shaders/deferred_shader.glfs", assets);
    PUSH_ASSET(SHADER, "deferred_shader.glvs", "res/shaders/deferred_shader.glvs", assets);
    PUSH_ASSET(SHADER, "fbotoscreen_shader.glfs", "res/shaders/fbotoscreen_shader.glfs", assets);
    PUSH_ASSET(SHADER, "gaussian_blur_shader.glfs", "res/shaders/gaussian_blur_shader.glfs", assets);
    PUSH_ASSET(SHADER, "geometry_vshader.glvs", "res/shaders/geometry_vshader.glvs", assets);
    PUSH_ASSET(SHADER, "glowthreshold_shader.glfs", "res/shaders/glowthreshold_shader.glfs", assets);
    PUSH_ASSET(SHADER, "light_fshader.glfs", "res/shaders/light_fshader.glfs", assets);
    PUSH_ASSET(SHADER, "selfillum_fshader.glfs", "res/shaders/selfillum_fshader.glfs", assets);
    PUSH_ASSET(SHADER, "shape_fshader.glfs", "res/shaders/shape_fshader.glfs", assets);
    PUSH_ASSET(SHADER, "textured_fshader.glfs", "res/shaders/textured_fshader.glfs", assets);

    //load sounds
    PUSH_ASSET(SOUND, "sfx_menu1", "res/sounds/sfx_menu1.wav", assets);
    PUSH_ASSET(SOUND, "sfx_jump1", "res/sounds/sfx_jump1.wav", assets);
    PUSH_ASSET(SOUND, "sfx_jump2", "res/sounds/sfx_jump2.wav", assets);
    PUSH_ASSET(SOUND, "sfx_jump3", "res/sounds/sfx_jump3.wav", assets);
    PUSH_ASSET(SOUND, "sfx_jump4", "res/sounds/sfx_jump4.wav", assets);
    PUSH_ASSET(SOUND, "sfx_death", "res/sounds/sfx_death.mp3", assets);

    //load sprites
    PUSH_ASSET(SPRITE, "spr_player", "res/sprites/spr_player.spr", assets);

    //load textures
    PUSH_ASSET(TEXTURE, "ascii", "res/textures/ascii.png", assets);
    PUSH_ASSET(TEXTURE, "blood", "res/textures/blood.png", assets);
    PUSH_ASSET(TEXTURE, "light_spot", "res/textures/light_spot.png", assets);
    PUSH_ASSET(TEXTURE, "medkit", "res/textures/medkit.png", assets);
    PUSH_ASSET(TEXTURE, "player", "res/textures/player.png", assets);
    PUSH_ASSET(TEXTURE, "prop_wall", "res/textures/prop_wall.png", assets);
    PUSH_ASSET(TEXTURE, "prop_wall2", "res/textures/prop_wall2.png", assets);
    PUSH_ASSET(TEXTURE, "tex_spike", "res/textures/tex_spike.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_city", "res/textures/tilesets/tileset_city.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_city_light", "res/textures/tilesets/tileset_city_light.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_city_mask", "res/textures/tilesets/tileset_city_mask.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_debug", "res/textures/tilesets/tileset_debug.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_debug_light", "res/textures/tilesets/tileset_debug_light.png", assets);
    PUSH_ASSET(TEXTURE, "tileset_debug_mask", "res/textures/tilesets/tileset_debug_mask.png", assets);
    PUSH_ASSET(TEXTURE, "tex_logo", "res/textures/tex_logo.png", assets);
    PUSH_ASSET(TEXTURE, "test_screenshot", "res/textures/test_screenshot.png", assets);
    PUSH_ASSET(TEXTURE, "rect_outline", "res/textures/tex_rect.png", assets);

    //load tilesets
    PUSH_ASSET(TILESET, "debug", "res/tilesets/debug.tsx", assets);
    PUSH_ASSET(TILESET, "tileset_city", "res/tilesets/tileset_city.tsx", assets);

    //load levels
    PUSH_ASSET(LEVEL, "testmap", "res/levels/testmap.tmx", assets);

    ///////////////////////////////////////////////////////////////////////////////////////////

    std::ofstream out("assets.rpak", std::ios::binary);

    PakHeader header = {};
    header.signature = PAK_SIGNATURE;
    header.version = PAK_VERSION;
    header.assetCount = assets.size();
    header.assetOffset = sizeof(PakHeader);

    out.write((const char*)&header, sizeof(PakHeader));

    std::vector<u8> fileContents;

    for (unsigned i = 0; i < header.assetCount; i++)
    {
        AssetData &assetData = assets.at(i);

        PakAsset asset = {};
        asset.type = (u32)assetData.type;
        asset.nameOffset = sizeof(PakAsset);
        asset.dataOffset = asset.nameOffset + assetData.name.size();
        asset.onePastDataOffset = asset.dataOffset + assetData.data.size();

        fileContents.insert(fileContents.end(), (u8*)&asset, ((u8*)&asset) + sizeof(asset));
        fileContents.insert(fileContents.end(), assetData.name.begin(), assetData.name.end());
        fileContents.insert(fileContents.end(), assetData.data.begin(), assetData.data.end());
/*
        out.write((const char*)&asset, sizeof(PakAsset));
        out.write(assetData.name.data(), assetData.name.size());
        out.write(assetData.data.data(), assetData.data.size());
*/
    }

    std::vector<u8> compressedData = Compression::compress(fileContents);

    out.write((const char*)compressedData.data(), compressedData.size());

    out.close();
    std::cout << "ASSETS PACKED!\n";
}

#define CHUNK_MAX_SIZE 32
#define CHUNK_MIN_SIZE 8

void split_binary_tree(TileBSPNode *root);

bool split_binary_tree_vertically(TileBSPNode *root, bool force = false)
{
    u32 width = root->get_width();
    u32 height = root->get_height();

    if (!force)
    {
        bool holeFound;
        //search right from center to find hole
        for(int x = width/2; x < width - CHUNK_MIN_SIZE; x++)
        {
            holeFound = true;
            for(int y = 0; y < height; y++)
            {
                if (root->get_tile(x,y) != 0)
                    holeFound = false;
            }
            if (holeFound)
            {
                std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") vertically! (x = " << x << ")\n";
                root->split_vertically(x);
                split_binary_tree(root->children);
                split_binary_tree(root->children + 1);
                std::cout << "Level " << root->level << " tree " << root << " splitting completed!\n";
                return true;
            }
        }
        //search left from center to find hole
        for(int x = width/2; x >= CHUNK_MIN_SIZE; x--)
        {
            holeFound = true;
            for(int y = 0; y < height; y++)
            {
                if (root->get_tile(x,y) != 0)
                    holeFound = false;
            }
            if (holeFound)
            {
                std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") vertically! (x = " << x << ")\n";
                root->split_vertically(x);
                split_binary_tree(root->children);
                split_binary_tree(root->children + 1);
                return true;
            }
        }

        return false;
    }
    else
    {
        std::cout << "Forcing vertical split!\n";
        std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") vertically! (x = " << width/2 << ")\n";
        root->split_vertically(width/2);
        split_binary_tree(root->children);
        split_binary_tree(root->children + 1);
        return true;
    }
}

bool split_binary_tree_horizontally(TileBSPNode *root, bool force = false)
{
    u32 width = root->get_width();
    u32 height = root->get_height();

    if (!force)
    {
        bool holeFound;
        //search down from center to find hole
        for(int y = height/2; y < height - CHUNK_MIN_SIZE; y++)
        {
            holeFound = true;
            for(int x = 0; x < width; x++)
            {
                if (root->get_tile(x,y) != 0)
                    holeFound = false;
            }
            if (holeFound)
            {
                std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") horizontally! (y = " << y << ")\n";
                root->split_horizontally(y);
                split_binary_tree(root->children);
                split_binary_tree(root->children + 1);
                return true;
            }
        }
        //search up from center to find hole
        for(int y = height/2; y >= CHUNK_MIN_SIZE; y--)
        {
            holeFound = true;
            for(int x = 0; x < width; x++)
            {
                if (root->get_tile(x,y) != 0)
                    holeFound = false;
            }
            if (holeFound)
            {
                std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") horizontally! (y = " << y << ")\n";
                root->split_horizontally(y);
                split_binary_tree(root->children);
                split_binary_tree(root->children + 1);
                return true;
            }
        }

        return false;
    }
    else
    {
        std::cout << "Forcing horizontal split!\n";
        std::cout << "Splitting a level " << root->level << " tree " << root << " (" << width << "x" << height << ") horizontally! (y = " << height/2 << ")\n";
        root->split_horizontally(height/2);
        split_binary_tree(root->children);
        split_binary_tree(root->children + 1);
        return true;
    }
}

void split_binary_tree(TileBSPNode *root)
{
    u32 width = root->get_width();
    u32 height = root->get_height();

    std::cout << "Attempting to split level " << root->level << " tree " << root << " (" << width << "x" << height << ")!\n";

    if (root->is_empty())
    {
        std::cout << "Level " << root->level << " tree " << root << " (" << width << "x" << height << ") is empty, not splitting!\n";
        return;
    }

    if (width >= CHUNK_MIN_SIZE || height >= CHUNK_MIN_SIZE)
    {
        //split vertically
        if (width >= height)
        {
            if (split_binary_tree_vertically(root))
            {
                return;
            }
            else if (split_binary_tree_horizontally(root))
            {
                return;
            }
            else if (width > CHUNK_MAX_SIZE)
            {
                split_binary_tree_vertically(root, true);
                return;
            }
            else
            {
                std::cout << "No need to split level " << root->level << " tree " << root << " (" << width << "x" << height << ") any further!\n";
            }
        }
        else //split horizontally
        {
            if (split_binary_tree_horizontally(root))
            {
                return;
            }
            else if (split_binary_tree_vertically(root))
            {
                return;
            }
            else if (height > CHUNK_MAX_SIZE)
            {
                split_binary_tree_horizontally(root, true);
                return;
            }
            else
            {
                std::cout << "No need to split level " << root->level << " tree " << root << " (" << width << "x" << height << ") any further!\n";
            }
        }
    }
    else
    {
        std::cout << "Cannot split level " << root->level << " tree " << root << " (" << width << "x" << height << ") any further!\n";
    }
}

void compile_level(const char *in, const char *out)
{
    std::cout << "COMPILING LEVEL...\n";

    rapidxml::file<> xmlFile(in);
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    LevelHeader header = {};
    header.signature = LEVEL_SIGNATURE;
    header.version = LEVEL_VERSION;
    header.nameOffset = sizeof(LevelHeader);

    rapidxml::xml_node<> *map = doc.first_node("map");

    header.width = atoi(map->first_attribute("width")->value());
    header.height = atoi(map->first_attribute("height")->value());

    std::vector<u32> tilesetOffsets;

    std::string levelName;

    std::cout << "1\n";

    //load properties
    rapidxml::xml_node<> *properties = map->first_node("properties");
    if (properties != NULL)
    {
        for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
        {
            std::string pname = property->first_attribute("name")->value();

            //figure out which property this is
            if (pname.compare("name") == 0)
            {
                levelName = property->first_attribute("value")->value();
                continue;
            }

            if (pname.compare("bgColor0") == 0)
            {
                std::stringstream ss;
                ss << property->first_attribute("value")->value();
                s32 r, g, b;
                ss >> r >> g >> b;
                //normalize and gamma correct
                header.bgColor1[0] = pow(r / 255.f, 2.2);
                header.bgColor1[1] = pow(g / 255.f, 2.2);
                header.bgColor1[2] = pow(b / 255.f, 2.2);
                header.bgColor1[3] = 1.0;
                continue;
            }

            if (pname.compare("bgColor1") == 0)
            {
                std::stringstream ss;
                ss << property->first_attribute("value")->value();
                s32 r, g, b;
                ss >> r >> g >> b;
                //normalize and gamma correct
                header.bgColor2[0] = pow(r / 255.f, 2.2);
                header.bgColor2[1] = pow(g / 255.f, 2.2);
                header.bgColor2[2] = pow(b / 255.f, 2.2);
                header.bgColor2[3] = 1.0;
                continue;
            }

            if (pname.compare("ambientColor") == 0)
            {
                std::stringstream ss;
                ss << property->first_attribute("value")->value();
                s32 r, g, b;
                ss >> r >> g >> b;
                //normalize and gamma correct
                header.ambientColor[0] = pow(r / 255.f, 2.2);
                header.ambientColor[1] = pow(g / 255.f, 2.2);
                header.ambientColor[2] = pow(b / 255.f, 2.2);
                header.ambientColor[3] = 1.0;
                continue;
            }

            //obviously, if there are multiples, they will overwrite the previous values
            //also, any properties not listed here will be ignored
            //it's also ok if none of these are present, cause we already initialized the struct with some values
        }
    }

    ///////////////////////////////////////////////////////////////////
    std::cout << "2\n";

    //a vector to temporarily store the body of the rlvl-file
    std::vector<u8> fileBody;

    u64 currentOffset = header.nameOffset;

    //write name to vector
    fileBody.insert(fileBody.end(), levelName.begin(), levelName.end());

    currentOffset += levelName.size();
    header.tilesetOffset = currentOffset;

    ////////////////////////////////////////////////////////////////////////////////
    std::cout << "3\n";

    //loop through tilesets and add them to the vector
    rapidxml::xml_node<> *tset = map->first_node("tileset");
    while (tset)
    {
        std::string tilesetFname = tset->first_attribute("source")->value();

        //process filename (remove res/ and .tsx)
        std::string prefix = "../tilesets/";
        std::string extension = ".tsx";

        memory_index i = tilesetFname.find(prefix);
        if (i != std::string::npos)
            tilesetFname.erase(i, prefix.length());

        memory_index j = tilesetFname.find(extension);
        if (j != std::string::npos)
            tilesetFname.erase(j, extension.length());

        ////////////////////////////////////////////////

        TilesetInfo tileset = {};
        tilesetOffsets.push_back(atoi(tset->first_attribute("firstgid")->value()));
        tileset.filenameOffset = currentOffset + sizeof(TilesetInfo);
        tileset.onePastFilenameOffset = tileset.filenameOffset + tilesetFname.size();

        fileBody.insert(fileBody.end(), (u8*)&tileset, ((u8*)&tileset) + sizeof(TilesetInfo));
        fileBody.insert(fileBody.end(), tilesetFname.begin(), tilesetFname.end());

        currentOffset = tileset.onePastFilenameOffset;
        tset = tset->next_sibling("tileset");
        header.tilesetAmount += 1;
    }

    header.tileLayerOffset = currentOffset;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "4\n";

    //loop through layers next
    for (rapidxml::xml_node<> *layer = map->first_node("layer"); layer; layer = layer->next_sibling("layer"))
    {
        TileLayerInfo tileLayer = {};
        //assign some initial values
        tileLayer.width = header.width;
        tileLayer.height = header.height;
        tileLayer.xScroll = 1.f;
        tileLayer.yScroll = 1.f;
        tileLayer.xTiling = false;
        tileLayer.yTiling = false;
        tileLayer.z = 0;
        tileLayer.collision = false;
        //tileLayer.tilesOffset = currentOffset + sizeof(TileLayerInfo);

        //loop through layer's properties
        rapidxml::xml_node<> *properties = layer->first_node("properties");
        if (properties)
        {
            for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
            {
                std::string pname = property->first_attribute("name")->value();

                //figure out which attribute this is
                if (pname.compare("width") == 0)
                {
                    tileLayer.width = atoi(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("height") == 0)
                {
                    tileLayer.height = atoi(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("depth") == 0)
                {
                    tileLayer.z = -atoi(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("z") == 0)
                {
                    tileLayer.z = atoi(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("xScroll") == 0)
                {
                    tileLayer.xScroll = atof(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("yScroll") == 0)
                {
                    tileLayer.yScroll = atof(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("xTiling") == 0)
                {
                    tileLayer.xTiling = atoi(property->first_attribute("value")->value()) != 0;
                    continue;
                }
                if (pname.compare("yTiling") == 0)
                {
                    tileLayer.yTiling = atoi(property->first_attribute("value")->value()) != 0;
                    continue;
                }
                if (pname.compare("collision") == 0)
                {
                    tileLayer.collision = atoi(property->first_attribute("value")->value()) != 0;
                    continue;
                }

                    //obviously, if there are multiples, they will overwrite the previous values
                    //also, any properties not listed here will be ignored
                    //it's also ok if none of these are present, cause we already initialized the struct with some values
            }
        }

        //tileLayer.onePastTilesOffset = tileLayer.tilesOffset + (sizeof(u32) * tileLayer.tileAmount);

        fileBody.insert(fileBody.end(), (u8*)&tileLayer, ((u8*)&tileLayer) + sizeof(TileLayerInfo));
        currentOffset += sizeof(TileLayerInfo);
        /*
        std::cout << "tileLayer written at " << currentOffset << std::endl
        << tileLayer.width << std::endl
        << tileLayer. height << std::endl
        << tileLayer.xSpeed << std::endl
        << tileLayer.ySpeed << std::endl
        << tileLayer.xTiling << std::endl
        << tileLayer.yTiling << std::endl
        << tileLayer.depth << std::endl
        << tileLayer.collision << std::endl
        << tileLayer.tileAmount << std::endl
        << tileLayer.tilesOffset << std::endl
        << tileLayer.onePastTilesOffset << std::endl;*/

        //u32 tileAmount = (tileLayer.width * tileLayer.height);
        //if (tileAmount == 0)
            //continue;

        rapidxml::xml_node<> *data = layer->first_node("data");

        //tileset groups
        for (int i = 0; i < tilesetOffsets.size(); i++)
        {
            //make a binary tree!
            TileBSPNode root(tileLayer.width,tileLayer.height);

            int row = 0, col = 0;

            int counter = 0;

            for (rapidxml::xml_node<> *tile = data->first_node("tile"); tile; tile = tile->next_sibling("tile"),col++,counter++)
            {
                if (col >= tileLayer.width)
                {
                    col = 0;
                    row++;
                }
                if (row >= tileLayer.height)
                    break;

                u32 t;
                rapidxml::xml_attribute<> *gid = tile->first_attribute("gid");
                if (gid)
                {
                    t = atoi(gid->value());
                    if (t >= tilesetOffsets[i])
                    {
                        if ((i+1 < tilesetOffsets.size()) && (t >= tilesetOffsets[i + 1]))
                        {
                            t = 0;
                        }
                        else
                        {
                            t -= (tilesetOffsets[i] - 1);
                        }
                    }
                    else
                    {
                        t = 0;
                    }
                }
                else t = 0;

                root.chunk[counter] = t;
            }

            std::cout << "Processing binary tree " << &root << " in tileset group " << i << std::endl;
            split_binary_tree(&root);

            u32 nodeCount = root.get_size();
            u32 treeDepth = root.get_depth();

            u32 fullTreeSize = 0;
            for (int i = 0; i <= treeDepth; i++)
            {
                fullTreeSize += std::pow(2,i);
            }
            std::cout << "Tree has " << nodeCount << " nodes and is " << treeDepth << " levels deep!\n";
            std::cout << "Full tree size is " << fullTreeSize << std::endl;

            int nodeDataIndices[fullTreeSize];
            std::vector<TileBSPNodeInfo> nodeInfo;
            std::vector<std::vector<u32>> chunkData;

            TileBSPTreeInfo treeInfo;
            treeInfo.nodeCount = nodeCount;
            treeInfo.fullTreeSize = fullTreeSize;

            std::cout << "Writing tree at position " << currentOffset << " (" << currentOffset - fileBody.size() << ")\n";

            fileBody.insert(fileBody.end(), (u8*)&treeInfo, ((u8*)&treeInfo) + sizeof(TileBSPTreeInfo));
            currentOffset += sizeof(TileBSPTreeInfo);

            for(int i=0; i<fullTreeSize; i++)
                nodeDataIndices[i] = -1;

            int skippedChunks = 0;
            skippedChunks = root.populate_arrays(nodeDataIndices, nodeInfo, chunkData, 0, skippedChunks);

            std::cout << "Indices: ";
            for (int i = 0; i < fullTreeSize; i++)
            {
                std::cout << nodeDataIndices[i] << ", ";
            }
            std::cout << std::endl;

            std::cout << "Writing indices at position " << currentOffset << " (" << currentOffset - fileBody.size() << ")\n";
            fileBody.insert(fileBody.end(), (u8*)nodeDataIndices, (u8*)nodeDataIndices + (sizeof(int) * fullTreeSize));
            currentOffset += (sizeof(int) * fullTreeSize);

            std::cout << "NodeInfo size is " << nodeInfo.size() << " (should be " << nodeCount << std::endl;

            for (int i = 0; i < nodeInfo.size(); i++)
            {
                int chunkOffset = nodeInfo.at(i).offset;

                std::cout << "Writing BSP Node at position " << currentOffset << ", tile count " << nodeInfo.at(i).tileCount << ", offset " << nodeInfo.at(i).offset << std::endl;
                fileBody.insert(fileBody.end(), (u8*)&nodeInfo.at(i), ((u8*)&nodeInfo.at(i)) + sizeof(TileBSPNodeInfo));
                currentOffset += sizeof(TileBSPNodeInfo);

                if (i-chunkOffset >= 0 && nodeInfo.at(i).tileCount > 0)
                {
                    std::cout << "Writing " << chunkData.at(i-chunkOffset).size() << " tiles! (" << i << ", " << i-chunkOffset << ") at position " << currentOffset << "\n";

                    fileBody.insert(fileBody.end(), (u8*)chunkData.at(i-chunkOffset).data(), (u8*)chunkData.at(i-chunkOffset).data() + (sizeof(u32) * chunkData.at(i-chunkOffset).size()));
                    currentOffset += (sizeof(u32) * chunkData.at(i-chunkOffset).size());
                }

            }
            std::cout << skippedChunks << " chunks skipped because they were empty!\n";
        }

        header.tileLayerAmount += 1;
    }

    header.entityOffset = currentOffset;

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "5\n";

    //finally, entities
    //here, object groups don't matter, they'll be combined
    for (rapidxml::xml_node<> *objGroup = map->first_node("objectgroup"); objGroup; objGroup = objGroup->next_sibling("objectgroup"))
    {
        for (rapidxml::xml_node<> *object = objGroup->first_node("object"); object; object = object->next_sibling("object"))
        {
            EntityInfo entity = {};
            entity.x = atoi(object->first_attribute("x")->value());
            entity.y = atoi(object->first_attribute("y")->value());
            if (object->first_attribute("width"))
                entity.w = atoi(object->first_attribute("width")->value());
            if (object->first_attribute("height"))
                entity.h = atoi(object->first_attribute("height")->value());
            entity.nameOffset = currentOffset + sizeof(EntityInfo);
            entity.depth = 0;



            //save properties to a temporary vector as well
            std::vector<std::pair<std::string, std::string>> propertyVector;

            rapidxml::xml_node<> *properties = object->first_node("properties");
            if (properties != NULL)
            {
                for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
                {
                    std::string pname = property->first_attribute("name")->value();
                    std::string ptype = property->first_attribute("value")->value();

                    if (pname.compare("depth") == 0)
                        entity.depth = atof(ptype.c_str());

                    propertyVector.emplace_back(pname, ptype);
                }
            }

            entity.propertyAmount = propertyVector.size();


            rapidxml::xml_attribute<> *type = object->first_attribute("type");
            rapidxml::xml_attribute<> *name = object->first_attribute("name");

            std::string entityName;
            if (name)
                entityName = name->value();

            std::string typeName;
            if (type)
                typeName = type->value();

            entity.typeOffset = entity.nameOffset + entityName.size();
            entity.propertyOffset = entity.typeOffset + typeName.size();

            currentOffset = entity.propertyOffset;

            std::vector<u8> propertyBin;
            for (auto &p : propertyVector)
            {
                EntityPropertyInfo entityProperty;
                entityProperty.nameOffset = currentOffset + sizeof(EntityPropertyInfo);
                entityProperty.valueOffset = entityProperty.nameOffset + p.first.size();
                entityProperty.onePastValueOffset = entityProperty.valueOffset + p.second.size();

                propertyBin.insert(propertyBin.end(), (u8*)&entityProperty, ((u8*)&entityProperty) + sizeof(EntityPropertyInfo));
                propertyBin.insert(propertyBin.end(), p.first.begin(), p.first.end());
                propertyBin.insert(propertyBin.end(), p.second.begin(), p.second.end());

                currentOffset = entityProperty.onePastValueOffset;
            }

            entity.onePastPropertyOffset = currentOffset - 1;

            fileBody.insert(fileBody.end(), (u8*)&entity, ((u8*)&entity) + sizeof(EntityInfo));
            fileBody.insert(fileBody.end(), entityName.begin(), entityName.end());
            fileBody.insert(fileBody.end(), typeName.begin(), typeName.end());

            fileBody.insert(fileBody.end(), propertyBin.begin(), propertyBin.end());
            header.entityAmount += 1;
        }
    }

    header.onePastEntityOffset = currentOffset;

    ///////////////////////////////////////////////////////////////////////////////////////
    std::cout << "6\n";

    //finally, write the whole thing into a file!!
    std::ofstream outFile(out, std::ios::binary);

    outFile.write((const char*)&header, sizeof(LevelHeader));
    outFile.write((const char*)fileBody.data(), fileBody.size());

    outFile.close();
    std::cout << "LEVEL COMPILED!\n";
}

void compile_tileset(const char *in, const char *out)
{
    std::cout << "COMPILING TILESET...\n";
    rapidxml::file<> xmlFile(in);
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    //////////////////////////////////////////////////

    rapidxml::xml_node<> *tileset = doc.first_node("tileset");

    std::string imgFname = "";
    std::string lightmapFname = "";
    std::string normalFname = "";
    std::string maskFname = "";

    u32 width = atoi(tileset->first_node("image")->first_attribute("width")->value());
    u32 height = atoi(tileset->first_node("image")->first_attribute("height")->value());

    rapidxml::xml_node<> *properties = tileset->first_node("properties");
    if (properties)
    {
        for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
        {
            std::string pname = property->first_attribute("name")->value();

            if (pname.compare("lightmap") == 0)
            {
                lightmapFname = property->first_attribute("value")->value();
                continue;
            }
            if (pname.compare("mask") == 0)
            {
                maskFname = property->first_attribute("value")->value();
                continue;
            }
            if (pname.compare("texture") == 0)
            {
                imgFname = property->first_attribute("value")->value();
                continue;
            }
            if (pname.compare("normal") == 0)
            {
                normalFname = property->first_attribute("value")->value();
                continue;
            }
        }
    }

    u32 tilesPerTex = (width * height) / (runnerTileSize * runnerTileSize);

    std::ofstream outFile(out, std::ios::binary);

    TilesetHeader header = {};
    header.signature = TILESET_SIGNATURE;
    header.version = TILESET_VERSION;
    header.tileCount = tilesPerTex;
    header.textureOffset = sizeof(TilesetHeader);
    header.lightmapOffset = header.textureOffset + imgFname.size();
    header.normalOffset = header.lightmapOffset + lightmapFname.size();
    header.tilesOffset = header.normalOffset + normalFname.size();

    outFile.write((const char*)&header, sizeof(TilesetHeader));

    outFile.write((const char*)imgFname.data(), imgFname.size());
    outFile.write((const char*)lightmapFname.data(), lightmapFname.size());
    outFile.write((const char*)normalFname.data(), normalFname.size());

    //parse masks
    std::vector<std::vector<u32>> masks;
    {
        std::stringstream file;
        file << "res/textures/" << maskFname << ".png";

        //push empty masks into vector
        for (u32 i = 0; i < tilesPerTex; i++)
        {
            std::vector<u32> mask;
            for (s32 j = 0; j < runnerTileSize; j++)
                mask.push_back(0);
            masks.push_back(mask);
        }

        //get image
        rImage maskImage = load_image(file.str().c_str());
        s32 imageSize = maskImage.width * maskImage.height;

        //loop thru pixels
        for (s32 i = 0; i < imageSize; i++)
        {
            //check alpha
            if (maskImage.pixels[(4 * i) + 3] == 0)
                continue;

            s32 xTile = (i % maskImage.width) / runnerTileSize;
            s32 yTile = (i / maskImage.width) / runnerTileSize;
            s32 x = (i % maskImage.width) % runnerTileSize;

            s32 tilesPerRow = maskImage.width / runnerTileSize;
            s32 tile = yTile * tilesPerRow + xTile;

            masks.at(tile).at(x) += 1;
        }

        delete_image(maskImage);
    }

    u32 tilesCreated = 0;
    for (rapidxml::xml_node<> *xmlTile = tileset->first_node("tile"); xmlTile; xmlTile = xmlTile->next_sibling("tile"))
    {
        u32 tileIndex = atoi(xmlTile->first_attribute("id")->value());

        //not all tiles are in the tileset, so put those in as empty tiles
        while (tileIndex > tilesCreated)
        {
            TileInfo emptyTile;

            emptyTile.type = 0;
            emptyTile.slope = 0;
            emptyTile.frameCount = 0;

            for (s32 i = 0; i < runnerTileSize; i++)
                emptyTile.mask[i] = masks.at(tilesCreated).at(i);

            outFile.write((const char*)&emptyTile, sizeof(emptyTile));
            tilesCreated++;
        }

        TileInfo tile;

        if (xmlTile->first_attribute("type"))
            tile.type = atoi(xmlTile->first_attribute("type")->value());
        else tile.type = 0;
        tile.slope = 0;
        tile.frameCount = 0;

        std::vector<TileAnimFrameInfo> animFrames;

        for (s32 i = 0; i < runnerTileSize; i++)
            tile.mask[i] = masks.at(tilesCreated).at(i);

        //loop through tile's properties
        rapidxml::xml_node<> *properties = xmlTile->first_node("properties");
        if (properties)
        {
            for (rapidxml::xml_node<> *property = properties->first_node("property"); property; property = property->next_sibling("property"))
            {
                std::string pname = property->first_attribute("name")->value();

                //figure out which attribute this is
                if (pname.compare("tileType") == 0)
                {
                    tile.type = atoi(property->first_attribute("value")->value());
                    continue;
                }
                if (pname.compare("slope") == 0)
                {
                    tile.slope = atof(property->first_attribute("value")->value());
                    continue;
                }

                //obviously, if there are multiples, they will overwrite the previous values
                //also, any properties not listed here will be ignored
                //it's also ok if none of these are present, cause we already initialized the struct with some values
            }
        }
        else
        {
            rapidxml::xml_node<> *anim = xmlTile->first_node("animation");
            if (anim)
            {
                for (rapidxml::xml_node<> *frame = anim->first_node("frame"); frame; frame = frame->next_sibling("frame"))
                {
                    tile.frameCount++;
                    TileAnimFrameInfo animFrame;

                    animFrame.frame = atoi(frame->first_attribute("tileid")->value());
                    animFrame.duration = atoi(frame->first_attribute("duration")->value());
                    animFrames.push_back(animFrame);
                }
                std::cout << outFile.tellp() << std::endl;
            }
        }
        outFile.write((const char*)&tile, sizeof(tile));
        outFile.write((const char*)animFrames.data(), animFrames.size() * sizeof(TileAnimFrameInfo));
        tilesCreated++;
    }
    while (tilesCreated < tilesPerTex)
    {
        TileInfo emptyTile;
        emptyTile.type = 0;
        emptyTile.slope = 0;
        for (s32 i = 0; i < runnerTileSize; i++)
            emptyTile.mask[i] = masks.at(tilesCreated).at(i);

        outFile.write((const char*)&emptyTile, sizeof(emptyTile));
        tilesCreated++;
    }

    outFile.close();

    std::cout << "TILESET COMPILED! Amount of tiles: " << tilesCreated << std::endl;
}

void compile_sprites()
{
    std::cout << "COMPILING SPRITES...\n";
    rapidxml::file<> xmlFile("res/sprites.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    //////////////////////////////////////////////////

    u32 spriteCount = 0;
    u64 currentOffset = 0;

    std::vector<u8> fileBody;

    for (rapidxml::xml_node<> *sprite = doc.first_node("sprite"); sprite; sprite = sprite->next_sibling("sprite"), spriteCount++)
    {
        SpriteInfo sInfo;

        sInfo.width = 0;
        sInfo.height = 0;
        sInfo.xoffset = 0.0;
        sInfo.yoffset = 0.0;
        sInfo.glow = 0.0;

        sInfo.animCount = 0;

        sInfo.nameOffset = currentOffset + sizeof(sInfo);

        std::string sName = "";
        std::string texName = "";
        std::string lmapName = "";
        std::string normalName = "";

        //find attributes
        for (rapidxml::xml_attribute<> *attr = sprite->first_attribute(); attr; attr = attr->next_attribute())
        {
            std::string attrName = attr->name();

            if (attrName.compare("width") == 0)
                sInfo.width = atoi(attr->value());

            else if (attrName.compare("height") == 0)
                sInfo.height = atoi(attr->value());

            else if (attrName.compare("xoffset") == 0)
                sInfo.xoffset = atof(attr->value());

            else if (attrName.compare("yoffset") == 0)
                sInfo.yoffset = atof(attr->value());

            else if (attrName.compare("glow") == 0)
                sInfo.glow = atof(attr->value());

            else if (attrName.compare("name") == 0)
                sName = attr->value();

            else if (attrName.compare("texture") == 0)
                texName = attr->value();

            else if (attrName.compare("lightmap") == 0)
                lmapName = attr->value();

            else if (attrName.compare("normal") == 0)
                normalName = attr->value();

        }

        sInfo.textureOffset = sInfo.nameOffset + sName.size();
        sInfo.lightmapOffset = sInfo.textureOffset + texName.size();
        sInfo.normalOffset = sInfo.lightmapOffset + lmapName.size();
        sInfo.animsOffset = sInfo.normalOffset + normalName.size();

        currentOffset = sInfo.animsOffset;

        //loop through animations
        std::vector<u8> animData;
        for (rapidxml::xml_node<> *anim = sprite->first_node("animation"); anim; anim = anim->next_sibling("animation"), sInfo.animCount++)
        {
            std::vector<u32> frames;

            //then loop through frames
            for (rapidxml::xml_node<> *frame = anim->first_node("frame"); frame; frame = frame->next_sibling("frame"))
            {
                u32 fIndex = atoi(frame->first_attribute("index")->value());
                std::cout << "frame index: " << fIndex << std::endl;
                frames.push_back(fIndex);
            }

            //get name
            std::string animName = "";
            rapidxml::xml_attribute<> *animNameAttr = anim->first_attribute("name");
            if (animNameAttr)
                animName = animNameAttr->value();

            SpriteAnimationInfo animInfo;
            animInfo.frameCount = frames.size();

            animInfo.nameOffset = currentOffset + sizeof(animInfo);
            animInfo.framesOffset = animInfo.nameOffset + animName.size();
            animInfo.onePastFramesOffset = animInfo.framesOffset + (frames.size() * sizeof(u32));

            //insert animation into vector as bytes
            animData.insert(animData.end(), (u8*)&animInfo, ((u8*)&animInfo) + sizeof(animInfo));
            animData.insert(animData.end(), animName.begin(), animName.end());
            animData.insert(animData.end(), (u8*)frames.data(), (u8*)frames.data() + (frames.size() * sizeof(decltype(frames)::value_type)));


            currentOffset = animInfo.onePastFramesOffset;
        }

        //insert one empty anim if there are none
        if (!sInfo.animCount)
        {
            SpriteAnimationInfo animInfo;
            animInfo.frameCount = 1;

            std::string defaultName = "default";
            u32 zero = 0;

            animInfo.nameOffset = currentOffset + sizeof(animInfo);
            animInfo.framesOffset = animInfo.nameOffset + defaultName.size();
            animInfo.onePastFramesOffset = animInfo.framesOffset + sizeof(zero);

            //insert animation into vector as bytes
            animData.insert(animData.end(), (u8*)&animInfo, ((u8*)&animInfo) + sizeof(animInfo));
            animData.insert(animData.end(), defaultName.begin(), defaultName.end());
            animData.insert(animData.end(), (u8*)&zero, (u8*)&zero + sizeof(zero));

            sInfo.animCount = 1;
        }

        sInfo.onePastAnimsOffset = currentOffset;

        //insert stuff
        fileBody.insert(fileBody.end(), (u8*)&sInfo, ((u8*)&sInfo) + sizeof(sInfo));
        fileBody.insert(fileBody.end(), sName.begin(), sName.end());
        fileBody.insert(fileBody.end(), texName.begin(), texName.end());
        fileBody.insert(fileBody.end(), lmapName.begin(), lmapName.end());
        fileBody.insert(fileBody.end(), normalName.begin(), normalName.end());
        fileBody.insert(fileBody.end(), animData.begin(), animData.end());

    }

    //write compiled file
    SpriteHeader header;
    header.signature = SPRITE_SIGNATURE;
    header.version = SPRITE_VERSION;
    header.spriteCount = spriteCount;
    header.spritesOffset = sizeof(header);

    std::ofstream outFile("res/sprites.rspr", std::ios::binary);
    outFile.write((const char*)&header, sizeof(header));
    outFile.write((const char*)fileBody.data(), fileBody.size());
    outFile.close();

    std::cout << "SPRITES COMPILED! Amount of sprites: " << spriteCount << std::endl;
}

std::vector<u8> load_mesh(std::string fname, ModelInfo *mInfo)
{
    std::vector<ModelVertInfo> vertList;
    std::vector<u32> indexList;

    std::ifstream file(fname);
    if (file.is_open())
    {
        std::vector<glm::vec3> vertPosList;
        std::vector<glm::vec2> texCoordList;
        std::vector<glm::vec3> normalList;

        //read file to the end
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);
            std::string firstWord;
            std::stringstream sstream(line);
            sstream >> firstWord;

            //load vertex coordinate
            if (firstWord.compare("v") == 0)
            {
                r32 x, y, z;
                sstream >> x >> y >> z;
                vertPosList.emplace_back(std::round(x), std::round(-y), std::round(-z));
            }
            //load texture coordinate
            else if (firstWord.compare("vt") == 0)
            {
                r32 u, v;
                sstream >> u >> v;
                texCoordList.emplace_back(u, 1 - v);
            }
            //load normals
            else if (firstWord.compare("vn") == 0)
            {
                r32 nx, ny, nz;
                sstream >> nx >> ny >> nz;
                normalList.emplace_back(nx,-ny,nz);
            }
            //load face
            else if (firstWord.compare("f") == 0)
            {
                std::string vert;
                u32 vertAmount = 0;
                u32 nextIndex = vertList.size();
                std::vector<u32> tempIndices;
                while (sstream >> vert)
                {
                    std::stringstream sstream2(vert);
                    u32 posIndex, texIndex, normIndex;
                    sstream2 >> posIndex;
                    sstream2.ignore(1);
                    sstream2 >> texIndex;
                    sstream2.ignore(1);
                    sstream2 >> normIndex;
                    posIndex--;
                    texIndex--;
                    normIndex--;

                    ModelVertInfo temp;
                    temp.x = vertPosList.at(posIndex).x;
                    temp.y = vertPosList.at(posIndex).y;
                    temp.z = vertPosList.at(posIndex).z;
                    temp.u = texCoordList.at(texIndex).x;
                    temp.v = texCoordList.at(texIndex).y;
                    temp.nx = -normalList.at(normIndex).x;
                    temp.ny = normalList.at(normIndex).y;
                    temp.nz = normalList.at(normIndex).z;

                    bool exists = false;
                    for (u32 i = 0; i < vertList.size(); i++)
                    {
                        if (vertList.at(i) == temp)
                        {
                            exists = true;
                            tempIndices.push_back(i);
                        }
                    }

                    if (!exists)
                    {
                        vertList.push_back(temp);
                        tempIndices.push_back(nextIndex++);
                    }
                    ++vertAmount;
                }
                //make triangle fan from polygon
                for (u32 i = 0; i < vertAmount - 2; i++)
                {
                    indexList.push_back(tempIndices.at(0));
                    indexList.push_back(tempIndices.at(i + 1));
                    indexList.push_back(tempIndices.at(i + 2));
                }
            }
        }
        file.close();
    }
    else std::cout << "poop :c\n";

    std::vector<u8> data;

    data.insert(data.end(), (u8*)vertList.data(), (u8*)vertList.data() + (vertList.size() * sizeof(decltype(vertList)::value_type)));
    data.insert(data.end(), (u8*)indexList.data(), (u8*)indexList.data() + (indexList.size() * sizeof(decltype(indexList)::value_type)));

    mInfo->indexOffset = mInfo->vertexOffset + (vertList.size() * sizeof(decltype(vertList)::value_type));
    mInfo->onePastIndexOffset = mInfo->indexOffset + (indexList.size() * sizeof(decltype(indexList)::value_type));

    std::cout << "mesh loaded!\n";
    return data;
}

void compile_models()
{
    std::cout << "COMPILING MODELS...\n";
    rapidxml::file<> xmlFile("res/models.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    //////////////////////////////////////////////////

    u32 modelCount = 0;
    u64 currentOffset = 0;

    std::vector<u8> fileBody;

    for (rapidxml::xml_node<> *model = doc.first_node("model"); model; model = model->next_sibling("model"), modelCount++)
    {
        ModelInfo mInfo;
        mInfo.glow = 0.0;

        std::string mName = "";
        std::string texName = "";
        std::string lmapName = "";
        std::string normalName = "";
        std::string meshFname = "";

        mInfo.nameOffset = currentOffset + sizeof(mInfo);

        //find attributes
        for (rapidxml::xml_attribute<> *attr = model->first_attribute(); attr; attr = attr->next_attribute())
        {
            std::string attrName = attr->name();

            if (attrName.compare("name") == 0)
                mName = attr->value();

            else if (attrName.compare("texture") == 0)
                texName = attr->value();

            else if (attrName.compare("lightmap") == 0)
                lmapName = attr->value();

            else if (attrName.compare("normal") == 0)
                normalName = attr->value();

            else if (attrName.compare("mesh") == 0)
                meshFname = attr->value();
        }

        mInfo.textureOffset = mInfo.nameOffset + mName.size();
        mInfo.lightmapOffset = mInfo.textureOffset + texName.size();
        mInfo.normalOffset = mInfo.lightmapOffset + lmapName.size();
        mInfo.vertexOffset = mInfo.normalOffset + normalName.size();

        std::vector<u8> meshData;
        meshData = load_mesh(meshFname, &mInfo);

        //insert stuff
        fileBody.insert(fileBody.end(), (u8*)&mInfo, ((u8*)&mInfo) + sizeof(mInfo));
        fileBody.insert(fileBody.end(), mName.begin(), mName.end());
        fileBody.insert(fileBody.end(), texName.begin(), texName.end());
        fileBody.insert(fileBody.end(), lmapName.begin(), lmapName.end());
        fileBody.insert(fileBody.end(), normalName.begin(), normalName.end());
        fileBody.insert(fileBody.end(), meshData.begin(), meshData.end());


        currentOffset = mInfo.onePastIndexOffset;
    }

    //write compiled file
    ModelHeader header;
    header.signature = MODEL_SIGNATURE;
    header.version = MODEL_VERSION;
    header.modelCount = modelCount;
    header.modelsOffset = sizeof(header);

    std::ofstream outFile("res/models.rmdl", std::ios::binary);
    outFile.write((const char*)&header, sizeof(header));
    outFile.write((const char*)fileBody.data(), fileBody.size());
    outFile.close();

    std::cout << "MODELS COMPILED! Amount of models: " << modelCount << std::endl;
}

void compile_particles()
{
    std::cout << "COMPILING PARTICLES...\n";
    rapidxml::file<> xmlFile("res/particles.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    //////////////////////////////////////////////////

    u32 systemCount = 0;
    u64 currentOffset = sizeof(ParticleHeader);

    std::vector<u8> fileBody;

    for (rapidxml::xml_node<> *system = doc.first_node("particlesystem"); system; system = system->next_sibling("particlesystem"), systemCount++)
    {
        u32 emitterCount = 0;

        std::string systemName;
        if (system->first_attribute("name"))
            systemName = system->first_attribute("name")->value();

        ParticleSystemInfo systemInfo;
        //systemInfo.emitterCount = emitterCount;
        systemInfo.nameOffset = currentOffset + sizeof(systemInfo);
        systemInfo.emitterOffset = systemInfo.nameOffset + systemName.size();

        currentOffset = systemInfo.emitterOffset;

        std::vector<u8> emitterVector;

        //loop through emitters
        for (rapidxml::xml_node<> *emitter = system->first_node("emitter"); emitter; emitter = emitter->next_sibling("emitter", emitterCount++))
        {
            std::string spritefName;
            ParticleEmitterInfo eInfo;
            eInfo.animation = 0;
            eInfo.spawnRate = 0;
            eInfo.duration = 0;
            eInfo.loops = 0;
            eInfo.gravity = 0;

            eInfo.spawnAreaType = 0;
            eInfo.lifeTime[0] = 0;
            eInfo.lifeTime[1] = 0;
            eInfo.initialScale[0] = 1;
            eInfo.initialScale[1] = 1;
            eInfo.initialVelocity[0] = 0;
            eInfo.initialVelocity[1] = 0;
            eInfo.initialVelocity[2] = 0;
            eInfo.initialVelocity[3] = 0;
            eInfo.initialRotation[0] = 0;
            eInfo.initialRotation[1] = 0;
            eInfo.animSpeed[0] = 0;
            eInfo.animSpeed[1] = 0;

            //find attributes
            for (rapidxml::xml_attribute<> *attr = emitter->first_attribute(); attr; attr = attr->next_attribute())
            {
                std::string attrName = attr->name();

                if (attrName.compare("sprite") == 0)
                    spritefName = attr->value();

                else if (attrName.compare("animation") == 0)
                    eInfo.animation = atoi(attr->value());

                else if (attrName.compare("spawnrate") == 0)
                    eInfo.spawnRate = atoi(attr->value());

                else if (attrName.compare("duration") == 0)
                    eInfo.duration = atoi(attr->value());

                else if (attrName.compare("loops") == 0)
                    eInfo.loops = atoi(attr->value());

                else if (attrName.compare("gravity") == 0)
                    eInfo.gravity = atoi(attr->value());
            }

            for (rapidxml::xml_node<> *node = emitter->first_node(); node; node = node->next_sibling())
            {
                std::string nodeName = node->name();

                if (nodeName.compare("spawnarea") == 0)
                {
                    for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        std::string attrName = attr->name();

                        if (attrName.compare("type") == 0)
                        {
                            std::string areaType = attr->value();

                            if (areaType.compare("point") == 0)
                                eInfo.spawnAreaType = 0;

                            else if (areaType.compare("rectangle") == 0)
                                eInfo.spawnAreaType = 1;

                            else if (areaType.compare("circle") == 0)
                                eInfo.spawnAreaType = 2;
                        }

                        else if (attrName.compare("radius") == 0)
                            *(r64*)eInfo.spawnAreaData = atof(attr->value());

                        else if (attrName.compare("dimensions") == 0)
                        {
                            r32 x, y;
                            std::stringstream sstream(attr->value());
                            sstream >> x >> y;
                            ((r32*)eInfo.spawnAreaData)[0] = x;
                            ((r32*)eInfo.spawnAreaData)[1] = y;
                        }
                    }
                }
                else
                {
                    std::string value, min, max;
                    for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        std::string attrName = attr->name();

                        if (attrName.compare("value") == 0)
                            value = attr->value();
                        else if (attrName.compare("min") == 0)
                            min = attr->value();
                        else if (attrName.compare("max") == 0)
                            max = attr->value();
                    }

                    std::stringstream valuestream(value), minstream(min), maxstream(max);


                    if (nodeName.compare("lifetime") == 0)
                    {
                        if (value.size())
                        {
                            valuestream >> eInfo.lifeTime[0];
                            eInfo.lifeTime[1] = eInfo.lifeTime[0];
                        }
                        else
                        {
                            minstream >> eInfo.lifeTime[0];
                            maxstream >> eInfo.lifeTime[1];
                        }
                    }

                    else if (nodeName.compare("initialscale") == 0)
                    {
                        if (value.size())
                        {
                            valuestream >> eInfo.initialScale[0];
                            eInfo.initialScale[1] = eInfo.initialScale[0];
                        }
                        else
                        {
                            minstream >> eInfo.initialScale[0];
                            maxstream >> eInfo.initialScale[1];
                        }
                    }

                    else if (nodeName.compare("initialvelocity") == 0)
                    {
                        if (value.size())
                        {
                            valuestream >> eInfo.initialVelocity[0] >> eInfo.initialVelocity[1];
                            eInfo.initialVelocity[2] = eInfo.initialVelocity[0];
                            eInfo.initialVelocity[3] = eInfo.initialVelocity[1];
                        }
                        else
                        {
                            minstream >> eInfo.initialVelocity[0] >> eInfo.initialVelocity[1];
                            maxstream >> eInfo.initialVelocity[2] >> eInfo.initialVelocity[3];
                        }
                    }

                    else if (nodeName.compare("initialrotation") == 0)
                    {
                        if (value.size())
                        {
                            valuestream >> eInfo.initialRotation[0];
                            eInfo.initialRotation[1] = eInfo.initialRotation[0];
                        }
                        else
                        {
                            minstream >> eInfo.initialRotation[0];
                            maxstream >> eInfo.initialRotation[1];
                        }
                    }

                    else if (nodeName.compare("animspeed") == 0)
                    {
                        if (value.size())
                        {
                            valuestream >> eInfo.animSpeed[0];
                            eInfo.animSpeed[1] = eInfo.animSpeed[0];
                        }
                        else
                        {
                            minstream >> eInfo.animSpeed[0];
                            maxstream >> eInfo.animSpeed[1];
                        }
                    }
                }
            }

            eInfo.spriteOffset = currentOffset + sizeof(eInfo);
            eInfo.onePastSpriteOffset = eInfo.spriteOffset + spritefName.size();
            //std::cout << eInfo.spriteOffset << ", " << eInfo.onePastSpriteOffset << std::endl;

            emitterVector.insert(emitterVector.end(), (u8*)&eInfo, ((u8*)&eInfo) + sizeof(eInfo));
            emitterVector.insert(emitterVector.end(), spritefName.begin(), spritefName.end());

            currentOffset = eInfo.onePastSpriteOffset;
        }

        systemInfo.emitterCount = emitterCount;
        systemInfo.onePastEmitterOffset = currentOffset;

        fileBody.insert(fileBody.end(), (u8*)&systemInfo, ((u8*)&systemInfo) + sizeof(systemInfo));
        fileBody.insert(fileBody.end(), systemName.begin(), systemName.end());
        fileBody.insert(fileBody.end(), emitterVector.begin(), emitterVector.end());
    }

    //write compiled file
    ParticleHeader header;
    header.signature = PARTICLE_SIGNATURE;
    header.version = PARTICLE_VERSION;
    header.systemCount = systemCount;
    header.systemOffset = sizeof(header);

    std::ofstream outFile("res/particles.rpar", std::ios::binary);
    outFile.write((const char*)&header, sizeof(header));
    outFile.write((const char*)fileBody.data(), fileBody.size());
    outFile.close();

    std::cout << "PARTICLE SYSTEMS COMPILED! Amount of systems: " << systemCount << std::endl;
}

void compile_multisounds()
{
    std::cout << "COMPILING MULTI SOUNDS...\n";
    rapidxml::file<> xmlFile("res/multisounds.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    //////////////////////////////////////////////////

    u32 msoundCount = 0;
    u64 currentOffset = sizeof(MultiSoundHeader);

    std::vector<u8> fileBody;

    for (rapidxml::xml_node<> *msound = doc.first_node("multisound"); msound; msound = msound->next_sibling("multisound"), msoundCount++)
    {
        std::string mSoundName;
        if (msound->first_attribute("name"))
            mSoundName = msound->first_attribute("name")->value();

        MultiSoundInfo mInfo;
        mInfo.nameOffset = currentOffset + sizeof(mInfo);
        mInfo.soundsOffset = mInfo.nameOffset + mSoundName.size();
        mInfo.soundCount = 0;

        currentOffset = mInfo.soundsOffset;

        std::vector<u8> soundVector;

        for (rapidxml::xml_node<> *sound = msound->first_node("sound"); sound; sound = sound->next_sibling("sound"), mInfo.soundCount++)
        {
            std::string soundName;
            if (sound->first_attribute("name"))
                soundName = sound->first_attribute("name")->value();

            MultiSoundSoundInfo sInfo;
            sInfo.nameOffset = currentOffset;
            sInfo.onePastNameOffset = currentOffset + soundName.size();

            currentOffset = sInfo.onePastNameOffset;

            soundVector.insert(soundVector.end(), (u8*)&sInfo, ((u8*)&sInfo) + sizeof(sInfo));
            soundVector.insert(soundVector.end(), soundName.begin(), soundName.end());
        }

        mInfo.onePastSoundsOffset = currentOffset;

        fileBody.insert(fileBody.end(), (u8*)&mInfo, ((u8*)&mInfo) + sizeof(mInfo));
        fileBody.insert(fileBody.end(), mSoundName.begin(), mSoundName.end());
        fileBody.insert(fileBody.end(), soundVector.begin(), soundVector.end());
    }

    //write compiled file
    MultiSoundHeader header;
    header.signature = MULTISOUND_SIGNATURE;
    header.version = MULTISOUND_VERSION;
    header.multiSoundCount = msoundCount;
    header.multiSoundsOffset = sizeof(header);

    std::ofstream outFile("res/multisounds.rsnd", std::ios::binary);
    outFile.write((const char*)&header, sizeof(header));
    outFile.write((const char*)fileBody.data(), fileBody.size());
    outFile.close();

    std::cout << "MULTISOUNDS COMPILED! Amount of systems: " << msoundCount << std::endl;
}


s32 main(s32 argc, char **argv)
{
    ilInit();
    build_assets();
    compile_tileset("res/tilesets/tileset_city.tsx", "res/tilesets/tileset_city.rtil");
    compile_tileset("res/tilesets/debug.tsx", "res/tilesets/debug.rtil");
    //compile_level("res/levels/testmap.tmx", "res/levels/testmap.rlvl");
    compile_level("res/levels/bigmap.tmx", "res/levels/bigmap.rlvl");
    compile_sprites();
    compile_models();
    compile_particles();
    compile_multisounds();
    ilShutDown();
    return 0;
}
