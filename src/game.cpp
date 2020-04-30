#include "game.h"
#include <sstream>
#include "Platform/platform_audio.h"
#include "units.h"
#include "input.h"
#include "renderer.h"
#include <algorithm>
#include "config.h"
#include "debug.h"
#include "resource.h"
#include "gamestate.h"
#include "macro.h"

Game::Game()
{
    Config::load_config();
    Renderer::init();
    Macro::init();
    Macro::load_playback_macro("savedMacro.rmac");
    //SoundManager::init();
    Resource::init();
    Resource::Loader::load_assets();

    std::cout << "Game <" << this << "> created!!\n";
}

Game::~Game()
{
    std::cout << "-Exiting main loop!-\n";
    Macro::save_current_macro("testmacro.rmac");
    platform_stop_music();
    Renderer::deinit();

    if (state)
        delete state;
    Resource::deinit();
}

void Game::init_game()
{
    //renderer.init_renderer(SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN*/, screenWidth / 16, screenHeight / 16);

    //EntitySystem::allocate_entity_data();

    //state = new LevelSelect();
    state = new MainMenu(0);
    //state = new GamePlay("res/testmap.tmx");

    //vsync = false;
    //platform_disable_vsync();
}

void Game::play(r64 dt)
{
    Renderer::clear_buffers();
    Renderer::update();

    //on F1, toggle vsync
    if (InputManager::get_input()->keyboard.keyF1 == KEY_JUST_PRESSED)
    {
        if (vsync)
            disable_vsync();
        else enable_vsync();
    }

    GameState *nextState = state->update();
    //SoundManager::update();
    platform_update_audio();
    draw_fps();
    Debug::print_records();

    if (nextState != state)
    {
        if (!nextState)
        {
            InputManager::get_input()->exit = true;
            return;
        }

        delete state;
        state = nextState;
    }

    Renderer::sort_buffers();

    fpstimer.add_time(dt);
}

void Game::draw(r32 timeRatio)
{
    //platform_unbind_framebuffer();
    platform_clear_buffer();

    state->draw(timeRatio);

    //platform_take_screenshot();
    platform_swap_buffer();
}

void Game::enable_vsync()
{
    platform_enable_vsync();
    vsync = true;
}
void Game::disable_vsync()
{
    platform_disable_vsync();
    vsync = false;
}

void Game::draw_fps()
{
    //draw fps
    std::stringstream fpscounter;
    r32 fps = fpstimer.get_fps();
    r64 dt = fpstimer.get_dt();
    fpscounter << std::round(fps) << " fps / " << std::round(dt) << "ms";
    Renderer::draw_string(fpscounter.str(), 0, 0, NULL, NULL, NULL, NULL);

    //draw vsync status
    if (vsync)
        Renderer::draw_string("vsync: on", 256, 0, NULL, NULL, NULL, NULL);
    else Renderer::draw_string("vsync: off", 256, 0, NULL, NULL, NULL, NULL);
}
