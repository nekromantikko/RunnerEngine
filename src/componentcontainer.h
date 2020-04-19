#ifndef COMPONENTCONTAINER_H
#define COMPONENTCONTAINER_H
#include "typedef.h"
#include "pool.h"

template <typename C>
class ComponentContainer
{
public:
    void allocate(u32 size)
    {
        components.allocate(size);
    }
    C *get_component(u32 id)
    {
        for (auto it = components.begin(); it != components.partition(); it++)
        {
            C *component = &(*it);
            if (component->get_ID() != id)
                continue;
            else return component;
        }
        return nullptr;
    }
    C *init_component()
    {
        C *component = components.init();

        return component;
    }
    void deinit_component(u32 id)
    {
        for (auto it = components.begin(); it != components.partition(); it++)
        {
            C *component = &(*it);
            if (component->get_ID() != id)
                continue;
            else components.deinit(component);
        }
    }
    void update_components()
    {
        for (auto it = components.begin(); it != components.partition(); it++)
            it->update();
    }
    void deinit_all()
    {
        components.deinit_all();
    }

private:
    Pool<C> components;
};

#endif // COMPONENTCONTAINER_H
