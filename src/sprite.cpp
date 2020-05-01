#include "sprite.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "resource.h"
#include "Platform/platform.h"

Sprite::~Sprite()
{
    std::cout << "Sprite <" << this << "> destroyed!" << std::endl;
}

void SpriteSheet::create_clipframes()
{
    //clipping
    r32 texLeft, texTop;

    if (texture)
    {
        //texture width / height in frames
        u32 texWidth;
        platform_get_texture_width(texture, &texWidth);
        u32 widthInFrames = texWidth / width;
        u32 texHeight;
        platform_get_texture_height(texture, &texHeight);
        u32 heightInFrames = texHeight / height;

        u32 frameCount = widthInFrames * heightInFrames;

        if (frameCount != 0)
        {
            clipFrames.reserve(frameCount);

            for (memory_index i = 0; i < frameCount; i++)
            {
                texLeft = (i % widthInFrames) / (r32)widthInFrames;
                texTop = (i / widthInFrames) / (r32)heightInFrames;

                r32 relativeWidth = (r32)width / texWidth;
                r32 relativeHeight = (r32)height / texHeight;
                v4 rect = {texLeft, texTop, relativeWidth, relativeHeight};
                clipFrames.push_back(rect);
            }
        }
    }
}

u32 SpriteSheet::get_frame(u32 anim, u32 index)
{
    u32 realAnim, realFrame;

    memory_index animAmount = animations.size();

    if (animAmount <= anim)
        realAnim = animAmount - 1;
    else realAnim = anim;

    memory_index animSize = animations.at(realAnim).frames.size();

    if (animSize <= index)
        realFrame = animSize - 1;
    else realFrame = index;

    return animations.at(realAnim).frames.at(realFrame);
}

v4 SpriteSheet::get_clip(u32 anim, u32 index)
{
    u32 frame = get_frame(anim, index);
    return get_clip(frame);
}

v4 SpriteSheet::get_clip(u32 frame)
{
    if (frame >= clipFrames.size())
    {
        if (clipFrames.empty())
            return {0,0,0,0};
        else return clipFrames.back();
    }
    else return clipFrames.at(frame);
}

Animation *SpriteSheet::get_anim(u32 anim)
{
    u32 realAnim;
    if (animations.size() <= anim)
        realAnim = animations.size() - 1;
    else realAnim = anim;

    return &animations.at(realAnim);
}

///////////////////////////////////////////////

void Sprite::update(r32 speed)
{
    //update animation
    accumulator += speed;

    while (accumulator >= 1)
    {
        accumulator -= 1;
        currentFrame++;
    }

    s32 animsize = sheet->get_anim(currentAnim)->frames.size();
    while (currentFrame >= animsize)
        currentFrame -= animsize;

}
void Sprite::set_animation (u32 animIndex)
{
    if (currentAnim == animIndex)
        return;

    currentAnim = animIndex;
    currentFrame = 0;
}
