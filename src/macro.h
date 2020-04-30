#ifndef MACRO_H
#define MACRO_H
#include "typedef.h"
#include <vector>
#include "signal.h"

//max macro length is 5 minutes
#define MACRO_MAX_LENGTH 18000

struct MacroInput
{
    r64 horState;
    r64 verState;
    r64 jumpState;
    r64 dashState;
};

struct MacroInputProcessed
{
    Signal xAxis;
    Signal yAxis;
    Signal jump;
    Signal dash;
};

enum MacroState
{
    MACRO_STOPPED = 0,
    MACRO_RECORDING = 1,
    MACRO_PLAYING = 2
};

namespace Macro
{
    void init();
    void deinit();
    void save_current_macro(const char *fname);
    void add_virtual_input(MacroInput input);
    bool load_playback_macro(const char *fname);
    void update();
    MacroState get_current_state();
    void play();
    void record();
    void stop();
}

#endif // MACRO_H
