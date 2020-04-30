#ifndef MESH_H
#define MESH_H

#include <vector>

struct Triangle
{
    u32 index[3];

    Triangle() {}
    Triangle(u32 a, u32 b, u32 c)
    {
        index[0] = a;
        index[1] = b;
        index[2] = c;
    }
};

struct InternalMesh;

struct MeshData
{
    //TODO: replace vectors with C arrays
    std::vector<v3> positions;
    std::vector<v2> texCoords;
    std::vector<v3> normals;
    std::vector<v4> tangents;

    std::vector<Triangle> tris;
};

struct Mesh
{
    InternalMesh *internal;
    MeshData data;
};

#endif // MESH_H
