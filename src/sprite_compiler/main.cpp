#include <fstream>
#include <iostream>
#include <string>
#include <IL/il.h>
#include <vector>
#include <sstream>
#include "..\compression.h"

struct spriteHeader
{
    char signature[4]; // 4 bytes
    r32 version; // 4
};

struct spriteData
{
    unsigned width; // 4
    unsigned height; // 4
    unsigned xoff; // 4
    unsigned yoff ; // 4
    r32 glow; // 4
};

struct fTexture
{
    std::string fname;
    unsigned width, height;
    unsigned *pixels;
};

std::fstream file, out;
std::vector<std::vector<unsigned>> anims;
spriteHeader header;
spriteData sprData;
fTexture diffuse, normal, lightmap;

void load_png(fTexture &texture)
{
    std::cout << texture.fname << std::endl;

    //generate and set current image ID
    ILuint imgID = 0;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    //load
    ILboolean success = ilLoadImage(texture.fname.c_str());

    //image loaded successfully
    if (success == IL_TRUE)
    {
        //convert into rgba
        success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

        if (success == IL_TRUE)
        {
            //create texture from pixels
            texture.width = ilGetInteger(IL_IMAGE_WIDTH);
            texture.height = ilGetInteger(IL_IMAGE_HEIGHT);
            texture.pixels = new unsigned[texture.width * texture.height];
            memcpy(texture.pixels, ilGetData(), texture.width * texture.height);
            std::cout << "texture loaded!\n";
        }

        //delete file from memory
        ilDeleteImages(1, &imgID);
    }
}


void read_data()
{
    //read file to the end
    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        std::string firstWord;
        std::stringstream sstream(line);
        sstream >> firstWord;

        //load vertex coordinate
        if (firstWord.compare("diffuse") == 0)
        {
            sstream >> diffuse.fname;
        }
        if (firstWord.compare("normal") == 0)
        {
            sstream >> normal.fname;
        }
        if (firstWord.compare("lightmap") == 0)
        {
            sstream >> lightmap.fname;
        }
        //load animation
        else if (firstWord.compare("anim") == 0)
        {
            std::vector<unsigned> anim;
            s32 index;
            while (sstream >> index)
                anim.push_back(index);

            anims.push_back(anim);
        }
        //load offsets
        else if (firstWord.compare("width") == 0)
            sstream >> sprData.width;
        else if (firstWord.compare("height") == 0)
            sstream >> sprData.height;
        else if (firstWord.compare("xoff") == 0)
            sstream >> sprData.xoff;
        else if (firstWord.compare("yoff") == 0)
            sstream >> sprData.yoff;
        else if (firstWord.compare("glow") == 0)
            sstream >> sprData.glow;
    }
}

s32 main(s32 argc, char **argv)
{
    //Initialize DevIL
    ilInit();
    ilClearColour( 255, 255, 255, 000 );

    std::string fname;

    std::cout << "enter filename\n";
    std::cin >> fname;

    file.open(fname.c_str());

    if (file.is_open())
    {
        header.signature[0] = 'S';
        header.signature[1] = 'P';
        header.signature[2] = 'R';
        header.signature[3] = 10;

        header.version = 1.0;

        sprData.width = 0;
        sprData.height = 0;
        sprData.xoff = 0;
        sprData.yoff = 0;
        sprData.glow = 0.0;

        read_data();

        file.close();

        diffuse.width = 0;
        diffuse.height = 0;
        normal.width = 0;
        normal.height = 0;
        lightmap.width = 0;
        lightmap.height = 0;

        if (diffuse.fname.size() != 0)
            load_png(diffuse);
        if (normal.fname.size() != 0)
            load_png(normal);
        if (lightmap.fname.size() != 0)
            load_png(lightmap);

        ////////////////////////////////////////////
        std::string fnameOut;
        std::cout << "enter filename for out\n";
        std::cin >> fnameOut;

        //write header to file
        out.open(fnameOut, std::fstream::out | std::fstream::trunc);

        out.write((char*)&header, sizeof(header));

        std::vector<u8> data;

        //write header to data
        for (unsigned i = 0; i < sizeof(sprData); i++)
            data.push_back(((char*)&sprData)[i]);


        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&diffuse.width)[i]);
        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&diffuse.height)[i]);
        for (unsigned i = 0; i < sizeof(unsigned) * diffuse.width * diffuse.height; i++)
            data.push_back(((char*)diffuse.pixels)[i]);

        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&normal.width)[i]);
        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&normal.height)[i]);
        for (unsigned i = 0; i < sizeof(unsigned) * normal.width * normal.height; i++)
            data.push_back(((char*)normal.pixels)[i]);

        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&lightmap.width)[i]);
        for (unsigned i = 0; i < sizeof(unsigned); i++)
            data.push_back(((char*)&lightmap.height)[i]);
        for (unsigned i = 0; i < sizeof(unsigned) * lightmap.width * lightmap.height; i++)
            data.push_back(((char*)lightmap.pixels)[i]);



        for (std::vector<unsigned> &anim : anims)
        {
            unsigned animSize = anim.size();
            for (unsigned i = 0; i < sizeof(unsigned); i++)
                data.push_back(((char*)&animSize)[i]);

            for (unsigned &frame : anim)
            {
                for (unsigned i = 0; i < sizeof(unsigned); i++)
                    data.push_back(((char*)&frame)[i]);
            }
        }

        //compress
        std::cout << "Compressing...\n";
        data = Compression::compress(data);
        if(data.size() == 0)
        {
            std::cout << "Failed to compress!\n";
            return 1;
        }

        //write compressed data to file
        out.write((char*)data.data(), data.size());


        std::cout << "done!\n";

        if(diffuse.pixels)
            delete[] diffuse.pixels;
        if(normal.pixels)
            delete[] normal.pixels;
        if(lightmap.pixels)
            delete[] lightmap.pixels;

        return 0;
    }

    return 1;
}
