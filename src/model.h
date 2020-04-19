#ifndef FMODEL_H
#define FMODEL_H
#include <string>
#include "shared.h"

struct Model
{
    ~Model();
    //bool load(std::string fname);

    Texture *texture;
    Texture *lightmap;
    Texture *normal;
    r32 glow;
    VertexArrayHandle *vbuffer;
};

#endif // FMODEL_H
