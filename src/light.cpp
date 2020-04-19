#include "light.h"
#include "renderer.h"
#include "entitymanager.h"

void LightComponent::update()
{
    Light light;
    light.position.xy() = parent->transform.position;
    light.position.xy() -= Renderer::get_camera_position();
    light.position.z = depth;
    light.color = color;

    if (state)
        Renderer::add_light(light, previous);

    previous = light;

}

void LightComponent::process_event(EntityEvent e)
{
    switch (e.type)
    {
    case TURN_ON:
        state = true;
        break;
    case TURN_OFF:
        state = false;
        break;
    default:
        break;
    }
}

void LightComponent::init()
{
}

void LightComponent::deinit()
{

}
