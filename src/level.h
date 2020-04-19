#ifndef LEVEL_H
#define LEVEL_H

#include <SDL.h>
#include <memory>
#include <list>
#include "leveltimer.h"
#include "vector.h"
#include "timer.h"

enum LevelState
{
    LEVEL_START,
    LEVEL_PLAYING,
    LEVEL_ENDING,
    LEVEL_ENDED,
    LEVEL_ERROR
};

class Level
{
public:
    Level();
    ~Level();
    Level (Level& other) = delete;
    Level& operator= (Level& other) = delete;
    void update();
    void draw(r32 drawRatio);
    bool is_loaded();
    void unload();
    void end_level(bool r);

    //drawing
    void draw_hud();

    void handle_collisions();
    bool load_level(std::string fname);
    std::string &get_filename() {return fileName;}
    s32 get_drum_channel() {return musicDrums;}

    LevelState get_state() const {return state;}

    u32 width, height;
    v2 camPos;

    bool32 drumsMuted = true;
private:
    //basic info
    std::string levelName, fileName;
    //background colors
    v3 bgColor[2];
    v3 ambientColor;
    ////////////////////
    LevelTimer timer;
    Timer transitionTimer;
    bool32 loaded = false;
    bool32 reload;
    bool32 musicPlaying = false;

    s32 musicBase = 0;
    s32 musicDrums = 0;
    LevelState state;

    r32 transitionRadius = 0;
};

namespace CurrentLevel
{
    void set_fname(std::string f);
    void load_level(std::atomic<s32> *done);
    void end_level(bool reload);
    bool level_loaded();
    void clear_entities_and_tiles();
    bool update_level();
    bool draw_level(r32);
    u32 get_width();
    u32 get_height();
    LevelState get_state();

    void unmute_drums();
    void mute_drums();
}


#endif // COLLISION_H
