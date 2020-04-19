#ifndef UTIL_SIGNAL_H
#define UTIL_SIGNAL_H

#include <iostream>
#include "typedef.h"
#include "math.h"
#define SENSITIVITY 0.1

///////////////////////////////////////////////////////////////////////////////
// class Signal
//==============
// A class representing a generic signal, for example wether a keyboard key
// is pressed or not, a window is shown or not, etc.
///////////////////////////////////////////////////////////////////////////////

enum SIGNAL_STATE
{
    LOW         = 0, HIGH,       FALLING_EDGE,       RISING_EDGE,
    BTN_UP      = 0, BTN_DOWN,   BTN_JUST_RELEASED,  BTN_JUST_PRESSED,
    KEY_UP      = 0, KEY_DOWN,   KEY_JUST_RELEASED,  KEY_JUST_PRESSED,
    OFF         = 0, ON,         JUST_TURNED_OFF,    JUST_TURNED_ON,
    SIG_FALSE   = 0, SIG_TRUE,   SIG_FALSE_WAS_TRUE, SIG_TRUE_WAS_FALSE
};

class Signal
{
public:
    Signal() : state_(0.f), prevState_(0.f) {}
    bool operator==(const SIGNAL_STATE &stateQuery) const
    {
        switch(stateQuery)
        {
        case LOW:
            return absolute_value(state_) <= SENSITIVITY;
            break;
        case HIGH:
            return absolute_value(state_) > SENSITIVITY;
            break;
        case FALLING_EDGE:
            return absolute_value(state_) <= SENSITIVITY && absolute_value(prevState_) > SENSITIVITY;
            break;
        case RISING_EDGE:
            return absolute_value(state_) > SENSITIVITY && absolute_value(prevState_) <= SENSITIVITY;
            break;
        }
        return false;
    }
    bool operator!=(const SIGNAL_STATE &stateQuery) const
    {
        return !((*this)==stateQuery);
    }
    void refresh()
    {
        prevState_ = state_;
    }
    const Signal &operator=(r64 assignedState)
    {
        state_ = assignedState;
        return *this;
    }
    r64 get_state() const
    {
        return state_;
    }

    r64 state_;
    r64 prevState_;
};

#endif // UTIL_SIGNAL_H
