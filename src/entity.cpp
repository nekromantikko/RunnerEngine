#include "entity.h"
#include "resource.h"
#include "Platform/platform.h"

void Entity::init()
{
    for (BehaviourComponent *behaviour : behaviours)
    {
        behaviour->init();
    }
}

void Entity::deinit()
{
    for (BehaviourComponent *behaviour : behaviours)
    {
        behaviour->deinit();
    }
    behaviours.clear();
    eventBuffer.clear();
}

void Entity::update()
{
    if (collider && ColliderComponent::visible)
        collider->draw(transform);

    for (BehaviourComponent *behaviour : behaviours)
    {
        //process events
        for (EntityEvent &event : eventBuffer)
            behaviour->process_event(event);

        behaviour->update();
    }

    eventBuffer.clear();
}

void Entity::update_render_component()
{
    if (render)
        render->update(transform, depth);
}

void Entity::add_event(EntityEvent e)
{
    eventBuffer.push_back(e);
}
void Entity::add_behaviour(BehaviourComponent *b)
{
    behaviours.push_back(b);
    behaviours.back()->parent = this;
}
u32 Entity::play_sound(std::string fname, s32 loops)
{
    //try finding a multisound
    MultiSound *mSound = Resource::get_multisound(fname);

    Sound *sound = NULL;
    if (mSound)
    {
        if (mSound->soundCount != 0)
        {
            u32 random = random_integer(0, mSound->soundCount - 1);
            sound = mSound->sounds[random];
        }
    }

    //if multisound wasn't found, try regular sound
    else sound = Resource::get_sound(fname);

    if (!sound)
        return 0;

    return platform_play_world_sound(sound, loops, &transform.position, nullptr);
}
void set_parent(Entity *entity)
{

}
