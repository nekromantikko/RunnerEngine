#ifndef LIGHT_H
#define LIGHT_H
#include "component.h"

class LightComponent : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    v4 color;
    r32 depth;
    bool32 state;
    Light previous;
};

#endif // LIGHT_H
