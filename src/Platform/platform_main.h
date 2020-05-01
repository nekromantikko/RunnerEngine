#ifndef PLATFORM_MAIN_H
#define PLATFORM_MAIN_H

#include "../signal.h"

struct ControllerInput
{
    Signal aButton;
    Signal bButton;
    Signal xButton;
    Signal yButton;
    Signal startButton;

    Signal xAxisLeft;
    Signal yAxisLeft;
    Signal triggerAxisRight;
};

struct KeyboardInput
{
    Signal keyLeft;
    Signal keyRight;
    Signal keyUp;
    Signal keyDown;
    Signal keySpace;
    Signal keyLeftShift;
    Signal keyEscape;
    Signal keyF1;
    Signal keyR;
    Signal keyD;
};

struct Input
{
    KeyboardInput keyboard;
    ControllerInput controller;
    bool exit;
};

/////////////////////////////

//init
void platform_init();
void platform_quit();
void platform_show_error(const char* error);

//time
r64 platform_time_in_ms();
u64 platform_get_ticks();
u64 platform_ms_to_ticks(r64 t);
r64 platform_ticks_to_ms(u64 t);
void platform_disable_vsync();
void platform_enable_vsync();

//input
void platform_init_controller();
void platform_deinit_controller();
bool platform_controller_exists();

void platform_poll_input(Input *input);

//haptic
void platform_controller_rumble(r32 strength, u32 lengthInMs);

void platform_set_fullscreen(bool);
void platform_swap_buffer();

#endif // PLATFORM_MAIN_H
