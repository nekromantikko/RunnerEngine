#ifndef LASER_BEAM_H
#define LASER_BEAM_H
#include "component.h"
#include "timer.h"

enum LaserState
{
    LASER_OFF,
    LASER_TURNING_ON,
    LASER_ON,
    LASER_TURNING_OFF
};

enum LaserType
{
    LASER_YELLOW,
    LASER_BLUE
};

class Laser : public BehaviourComponent
{
public:
    void update();
    inline void update_yellow(Entity *parent);
    inline void update_blue(Entity *parent);
    void process_event(EntityEvent e);
    void init();
    void deinit();
    Timer stateChangeTimer;
    LaserState state = LASER_OFF;
    LaserType type = LASER_YELLOW;
};
#endif // LASER_BEAM_H
