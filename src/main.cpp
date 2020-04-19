#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <memory>
#include "game.h"
#include <thread>

#include "Platform/platform.h"
#include "input.h"
#include "entitymanager.h"
#include "units.h"
#include "debug.h"

Game *game;

extern "C" void runner_initialize()
{
    //init_libraries();
    platform_init();
    game = new Game();
    game->init_game();
}

extern "C" void runner_play()
{
    u64 accumulator = 0;
    const u64 step = platform_ms_to_ticks(1000.f/60);
    u64 currentTime = platform_get_ticks();

    try
    {
        while(!InputManager::get_input()->exit)
        {
            u64 newTime = platform_get_ticks();
            u64 frameTime = newTime - currentTime;
            currentTime = newTime;

            accumulator += frameTime;
            while (accumulator >= step)
            {
                InputManager::refresh();
                platform_poll_input(InputManager::get_input());
                game->play(platform_ticks_to_ms(frameTime));
                accumulator -= step;
            }

            r32 alpha = accumulator / step;
            game->draw(alpha);
        }
    }
    catch (std::runtime_error error)
    {
        platform_show_error(error.what());
    }
}

s32 main(s32 argc, char **argv)
{
    EntityManager::allocate(1024);
    runner_initialize();

    runner_play();

    delete game;
    platform_quit();

    return(0);
}
