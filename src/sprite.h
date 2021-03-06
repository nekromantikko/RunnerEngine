#ifndef FSPRITE_H
#define FSPRITE_H
#include "typedef.h"
#include <vector>
#include <string>
#include "vector.h"

class Texture;

struct Animation
{
    Animation(std::string n) : name(n)
    {}
    std::vector<u32> frames;
    std::string name;
};

//a class that will be opened from file
struct Sprite
{
    Texture *texture;
    Texture *lightmap;
    Texture *normal;
    std::vector<Animation> animations;
    std::vector<v4> clipFrames;

    u32 width, height;
    r32 xOffset, yOffset;
    r32 glow;

    ~Sprite();
    void create_vertex_buffer();
    void create_clipframes();
    u32 get_frame(u32 anim, u32 index);
    v4 get_clip(u32 anim, u32 index);
    v4 get_clip(u32 frame);
    Animation *get_anim(u32 anim);
};

struct SpriteInstance
{
    void update(r32 speed);
    void set_animation (u32 animIndex);

    Sprite *sprite;
    //animstuff
    r32 accumulator = 0;
    u32 currentAnim = 0, currentFrame = 0;
};

#endif // FSPRITE_H
