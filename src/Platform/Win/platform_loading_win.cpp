#include "../platform_loading.h"


rImage platform_load_image(const char* fname)
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
void platform_delete_image(rImage image)
{
    delete[] image.pixels;
}

void platform_load_palette(const char *fname, void *output, int s = 16)
{
    FILE *file = fopen(fname, "r");
    if (file != NULL)
    {
        char id1[9];
        char id2[5];
        char colorCountStr[8];

        fscanf(file, "%s9", id1);
        fscanf(file, "%s5", id2);
        fscanf(file, "%s8", colorCountStr);

        int colorCount = atoi(colorCountStr);
        colorCount = std::min(s, colorCount);

        char r[4];
        char g[4];
        char b[4];

        for (int i = 0; i < colorCount; i++)
        {
            fscanf(file, "%s4", r);
            fscanf(file, "%s4", g);
            fscanf(file, "%s4", b);

            //gamma correct
            u8 r_gamma, g_gamma, b_gamma;
            r_gamma = std::pow((atoi(r) / 255.f), 2.2) * 255.f;
            g_gamma = std::pow((atoi(g) / 255.f), 2.2) * 255.f;
            b_gamma = std::pow((atoi(b) / 255.f), 2.2) * 255.f;

            u8 r5, g6, b5;
            r5 = (r_gamma >> 3);
            g6 = (g_gamma >> 2);
            b5 = (b_gamma >> 3);

            u8 *data = (u8*)output + i*2;

            u8 hi, lo;
            hi = (g6 >> 3) + (r5 << 3);
            lo = (g6 << 5) + b5;

            data[0] = lo;
            data[1] = hi;


        }

    }
}

Mesh platform_load_mesh(const char* fname)
{
    Mesh mesh;

    std::ifstream file(fname);
    if (file.is_open())
    {
        std::vector<v3> vertPosList;
        std::vector<v2> texCoordList;
        std::vector<v3> normalList;

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
                vertPosList.emplace_back(std::round(x), std::round(-y), std::round(z));
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
                normalList.emplace_back(nx,ny,nz);
            }
            //load face
            else if (firstWord.compare("f") == 0)
            {
                std::string vert;
                u32 vertAmount = 0;
                u32 nextIndex = mesh.positions.size();
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

                    v3 tempPos = {vertPosList.at(posIndex).x, vertPosList.at(posIndex).y, vertPosList.at(posIndex).z};
                    v2 tempCoord = {texCoordList.at(texIndex).x, texCoordList.at(texIndex).y};
                    v3 tempNormal = {normalList.at(normIndex).x, normalList.at(normIndex).y, normalList.at(normIndex).z};

                    bool exists = false;
                    for (u32 i = 0; i < mesh.positions.size(); i++)
                    {
                        if (mesh.positions.at(i) == tempPos && mesh.texCoords.at(i) == tempCoord && mesh.normals.at(i) == tempNormal)
                        {
                            exists = true;
                            tempIndices.push_back(i);
                        }
                    }

                    if (!exists)
                    {
                        mesh.positions.push_back(tempPos);
                        mesh.texCoords.push_back(tempCoord);
                        mesh.normals.push_back(tempNormal);

                        tempIndices.push_back(nextIndex++);
                    }
                    ++vertAmount;
                }
                //make triangle fan from polygon
                for (u32 i = 0; i < vertAmount - 2; i++)
                    mesh.tris.emplace_back(tempIndices.at(0), tempIndices.at(i + 1), tempIndices.at(i + 2));
            }
        }
        file.close();
    }
    else
    {
        std::stringstream errormsg;
        errormsg << "Cannot load obj-file " << fname;
        platform_show_error(errormsg.str().c_str());
    }

    platform_calculate_tangent(&mesh);

    return mesh;
}

void platform_calculate_tangent(Mesh *mesh)
{
    u32 vertexCount = mesh->positions.size();

    v3 *tan1 = new v3[vertexCount * 2];
    v3 *tan2 = tan1 + vertexCount;
    memset(tan1, 0, vertexCount * 2 * sizeof(v3));

    //for each triangle in mesh
    for (u32 i = 0; i < mesh->tris.size(); i++)
    {
        Triangle *triangle = &mesh->tris.at(i);
        u32 i1 = triangle->index[0];
        u32 i2 = triangle->index[1];
        u32 i3 = triangle->index[2];

        const v3 vert1 = mesh->positions.at(i1);
        const v3 vert2 = mesh->positions.at(i2);
        const v3 vert3 = mesh->positions.at(i3);

        const v2 uv1 = mesh->texCoords.at(i1);
        const v2 uv2 = mesh->texCoords.at(i2);
        const v2 uv3 = mesh->texCoords.at(i3);

        r32 x1 = vert2.x - vert1.x;
        r32 x2 = vert3.x - vert1.x;
        r32 y1 = vert2.y - vert1.y;
        r32 y2 = vert3.y - vert1.y;
        r32 z1 = vert2.z - vert1.z;
        r32 z2 = vert3.z - vert1.z;

        r32 s1 = uv2.x - uv1.x;
        r32 s2 = uv3.x - uv1.x;
        r32 t1 = uv2.y - uv1.y;
        r32 t2 = uv3.y - uv1.y;

        r32 r = 1.f / (s1 * t2 - s2 * t1);
        v3 sdir({(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r});
        v3 tdir({(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r});

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (u32 i = 0; i < vertexCount; i++)
    {
        const v3 &n = mesh->normals.at(i);
        const v3 &t = tan1[i];

        v4 tangent;
        //Gram-Schmidt orthogonalize
        tangent.xyz() = Normalize(t - n * Inner(n, t));

        //Calculate handedness
        tangent.w = (Inner(Cross(n, t), tan2[i]) < 0.0F) ? -1.0F : 1.0F;

        mesh->tangents.push_back(tangent);
    }

    delete[] tan1;
}

Sound *platform_load_sound(const char* fname)
{
    /*Mix_Chunk *chunk = Mix_LoadWAV(fname);
    if (chunk)
    {
        Sound *sound = new Sound;
        sound->chunk = chunk;
        return sound;
    }

    return NULL;*/
    FMOD_RESULT result;
    Sound *sound = new Sound;
    result = FMOD_System_CreateSound(soundSystem, fname, FMOD_CREATESAMPLE | FMOD_3D | FMOD_LOOP_NORMAL | FMOD_3D_LINEARSQUAREROLLOFF, NULL, &sound->sound);

    if (result == FMOD_OK)
        return sound;

    delete sound;
    return NULL;
}
void platform_delete_sound(Sound *sound)
{
    //Mix_FreeChunk(sound->chunk);
    //delete sound;
    if (sound)
    {
        FMOD_Sound_Release(sound->sound);
        delete sound;
    }
}

Music *platform_load_music(const char* fname)
{
    /*Music *music = new Music;
    music->music = Mix_LoadMUS(fname);
    if (!music->music)
        std::cout << "problem loading " << fname << ": " << Mix_GetError() << std::endl;
    return music;*/
    FMOD_RESULT result;
    Music *music = new Music;
    result = FMOD_System_CreateSound(soundSystem, fname, FMOD_CREATESTREAM | FMOD_2D | FMOD_LOOP_NORMAL, NULL, &music->sound);

    if (result == FMOD_OK)
        return music;

    delete music;
    return NULL;
}
void platform_delete_music(Music *music)
{
    /*Mix_FreeMusic(music->music);
    delete music;*/
    if (music)
    {
        FMOD_Sound_Release(music->sound);
        delete music;
    }
}
