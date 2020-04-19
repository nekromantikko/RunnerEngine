#ifndef HUDCOMPONENT_H
#define HUDCOMPONENT_H
#include "component.h"

class PlayerHUD : public BehaviourComponent
{
public:
    void update();
    void process_event(EntityEvent e);
    void init();
    void deinit();

    u32 *health;
private:
    Sprite *sprite;
    r32 pump;
    r32 pumpSpeed;
};

#endif // HUDCOMPONENT_H
