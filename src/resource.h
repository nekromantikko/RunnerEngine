#ifndef RESOURCE_H
#define RESORUCE_H

namespace Resource
{
    void init();
    void deinit();

    void load_assets();
    void free_assets();

    u8 load_internal_texture(const char *name, const char *fname);
    InternalTexture *get_internal_texture(const char *name);
    InternalTexture *get_internal_texture(u8 i);
    u8 get_internal_texture_index(const char *name);

    u8 load_internal_shader(const char *name, const char *vert, const char *frag);
    InternalShader *get_internal_shader(const char *name);
    InternalShader *get_internal_shader(u8 i);
    u8 get_internal_shader_index(const char *name);

    u8 load_internal_mesh(const char *name, const char *fname);
    InternalMesh *get_internal_mesh(const char *name);
    InternalMesh *get_internal_nesh(u8 i);
    u8 get_internal_mesh_index(const char *name);
}


#endif // DATABASES_H

