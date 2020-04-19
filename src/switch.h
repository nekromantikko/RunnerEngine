#ifndef SWITCH_H
#define SWITCH_H
#include "component.h"
#include "timer.h"

class Switch : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();
    Timer stateChangeTimer;
    bool32 state = 0;
    bool32 colliding = false;
    u32 time = 0;
    CollisionFlag flags = COLLISION_NONE;
    std::string targetName = "";
};

#endif // SWITCH_H
