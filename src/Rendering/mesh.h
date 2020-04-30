#ifndef MESH_H
#define MESH_H

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
    v3 *position;
    v2 *texCoord;
    v3 *normal;
    v4 *tangent;

    Triangle *tris;
};

struct Mesh
{
    InternalMesh *internal;
    MeshData data;
};

#endif // MESH_H
