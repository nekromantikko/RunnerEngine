#ifndef PLATFORM_LOADING_H
#define PLATFORM_LOADING_H

struct rImage
{
    u8 *pixels;
    u32 width, height;
};

rImage platform_load_image(const char* fname);
void platform_delete_image(rImage image);

void platform_load_palette(const char *fname, void *output, int s);

Mesh platform_load_mesh(const char* fname);
void platform_calculate_tangent(Mesh *mesh);

Sound *platform_load_sound(const char* fname);
void platform_delete_sound(Sound *sound);
Music *platform_load_music(const char* fname);
void platform_delete_music(Music *music);

#endif // PLATFORM_MAIN_H
