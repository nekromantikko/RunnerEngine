#ifndef GAME_H
#define GAME_H

#include "fpstimer.h"

class GameState;

class Game
{
public:
    Game();
    ~Game();
    void init_game();
    void play(r64);
    //returns drawing time
    void draw(r32 timeRatio);
    void quit();
    void enable_vsync();
    void disable_vsync();
    //draw fps and other info on screen
    void draw_fps();
private:
    bool vsync = true;
    GameState *state;
    FPSTimer fpstimer;
};

#endif // GAME_H
