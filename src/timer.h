#ifndef TIMER_H
#define TIMER_H
#include <functional>
#include "typedef.h"

class Timer
{
public:
    Timer();
    Timer(std::function<void()> call);
    ~Timer();
    void set_timer(u32 t);
    void operator= (u32);
    bool operator== (u32 n);
    bool operator!= (u32 n);
    bool operator< (u32 n);
    bool operator> (u32 n);
    bool operator>= (u32 n);
    bool operator<= (u32 n);
    u32 operator% (u32 n);
    void stop_timer();
    void continue_timer();
    void reset();
    bool was_just_stopped();
    bool is_stopped();
    bool is_running();
    bool was_just_started();
    bool get_state() const ;
    u32 get_time() const;
    //return state
    bool update();
private:
    u32 time;
    bool state, prevState;
    std::function<void()> callBack;
};

#endif // TIMER_H
