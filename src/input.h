#ifndef INPUT_H
#define INPUT_H
#include "shared.h"
enum InputAxis
{
    AXIS_RIGHT,
    AXIS_UP
};

enum InputAction
{
    JUMP,
    DASH,
    PAUSE,
    SHOOT
};

struct MacroInputProcessed;

namespace InputManager
{
    const r64 axis(InputAxis axis);
    const bool axis_moved(InputAxis axis);
    const bool axis_released(InputAxis axis);

    const bool button(InputAction action);
    const bool button_down(InputAction action);
    const bool button_up(InputAction action);
    Input *get_input();
    void refresh();
    void set_macro_input(MacroInputProcessed input);
}

#endif // INPUT_H
