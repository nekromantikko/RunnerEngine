#include "hudcomponent.h"
#include "resource.h"
#include "renderer.h"

void PlayerHUD::update()
{
    pumpSpeed = 0.25 - *health * 0.05;
    pump += pumpSpeed;

    for (int i = 0; i < 3; i++)
    {
        Transform xform;
        xform.position = {(f32)32 + 34*i, (f32)32};
        xform.scale.xy() = {32, 32};

        u32 frame;
        if (*health <= i)
            frame = 0;
        else
        {
            frame = 1;
            if (*health == i + 1)
            {
                r32 scale = 1 + (0.1 * sin(pump));
                xform.scale *= scale;
            }
        }

        UICall call;
        call.a = xform;
        call.b = xform;
        call.texture = sprite->texture;
        call.clipRect = sprite->get_clip(frame);
        call.offset = {sprite->xOffset, sprite->yOffset};
        call.color = {1,1,1,1};
        call.flip = {0,0};
        call.priority = 999;

        Renderer::add_ui_element(call);
    }

}
void PlayerHUD::process_event(EntityEvent e)
{

}
void PlayerHUD::init()
{
    pump = 0;

    Rectangle2 fullRect = {16, 32, 0, 16};
    Rectangle2 emptyRect = {0, 16, 0, 16};

    sprite = Resource::get_sprite("spr_heart");
}
void PlayerHUD::deinit()
{

}
