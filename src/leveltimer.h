#ifndef LEVELTIMER_H
#define LEVELTIMER_H

#include "typedef.h"

//a class to keep track of passed time in a level, and also fps
class LevelTimer
{
public:
    LevelTimer();
    ~LevelTimer();
    void update();
    s32 get_minutes();
    s32 get_seconds();
    s32 get_centiseconds();
    void init();
private:
    //amount of time passed, in milliseconds
    r64 time;
    r64 startTime;
};

#endif // LEVELTIMER_H
