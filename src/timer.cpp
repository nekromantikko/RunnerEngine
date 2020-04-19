#include "timer.h"

Timer::Timer() : time(0), callBack(nullptr)
{
    state = 0;
    prevState = 0;
}

Timer::Timer(std::function<void()> call) : time(0), callBack(call)
{
    state = 0;
    prevState = 0;
}

Timer::~Timer()
{
}

void Timer::set_timer(u32 t)
{
    time = t;
    continue_timer();
}

void Timer::operator= (u32 n)
{
    set_timer(n);
}

bool Timer::operator== (u32 n)
{
    return time == n;
}

bool Timer::operator!= (u32 n)
{
    return time != n;
}

bool Timer::operator< (u32 n)
{
    return time < n;
}

bool Timer::operator> (u32 n)
{
    return time > n;
}

bool Timer::operator>= (u32 n)
{
    return time >= n;
}

bool Timer::operator<= (u32 n)
{
    return time <= n;
}

u32 Timer::operator% (u32 n)
{
    return time % n;
}

void Timer::stop_timer()
{
    prevState = state;
    state = 0;
}

void Timer::continue_timer()
{
    prevState = state;
    state = 1;
}

void Timer::reset()
{
    time = 0;
    prevState = 0;
    state = 0;
}

bool Timer::was_just_stopped()
{
    return (state == 0 && prevState == 1);
}

bool Timer::is_stopped()
{
    return state == 0;
}

bool Timer::is_running()
{
    return state == 1;
}

bool Timer::was_just_started()
{
    return (state == 1 && prevState == 0);
}

bool Timer::get_state() const
{
    return state;
}

u32 Timer::get_time() const
{
    return time;
}

bool Timer::update()
{
    prevState = state;

    if (state == 1)
    {
        time--;
        if (time == 0)
        {
            //stop timer and call back
            state = 0;
            if (callBack)
                callBack();
        }
    }

    return state;
}
