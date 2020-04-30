#include "gamestate.h"
#include "level.h"
#include "world.h"
#include "input.h"
#include <sstream>
#include "Platform/platform.h"
#include "entitymanager.h"
#include "renderer.h"
#include "config.h"
#include "resource.h"
#include "macro.h"
#include <atomic>

GenericMenu::GenericMenu(s32 x, s32 y, s32 w, s32 h, std::vector<const char*> i, u32 pos) : xPos(x), yPos(y), width(w), height(h), items(i), currentChoice(pos)
{
}

u8 GenericMenu::update()
{
    //move cursor
    r64 yAxis = InputManager::axis(AXIS_UP);

    if (yAxis && buttonDown == false)
    {
        if (yAxis > 0 && currentChoice > 0)
        {
            platform_play_menu_sound(Resource::get_sound("sfx_menu1"), 0);
            currentChoice -= 1;
        }
        else if (yAxis < 0 && currentChoice < (items.size() - 1))
        {
            platform_play_menu_sound(Resource::get_sound("sfx_menu1"), 0);
            currentChoice += 1;
        }
        buttonDown = true;
    }

    if (InputManager::axis_released(AXIS_UP))
        buttonDown = false;

    //draw box
    v4 black = {0,0,0,1};
    Renderer::draw_rectangle(xPos, yPos, width, height, NULL, &black);

    //draw arrow
    v4 yellow = {1,1,0,1};
    Renderer::draw_string("\x82", xPos + 16, yPos + 32 + 48 * currentChoice, NULL, &yellow, NULL, NULL);

    //draw menu items (do this better later)
    for (u32 i = 0; i < items.size(); i++)
    {
        if (currentChoice == i)
            Renderer::draw_string(items.at(i), xPos + 48, yPos + 32 + 48 * i, NULL, &yellow, NULL, NULL);
        else Renderer::draw_string(items.at(i), xPos + 48, yPos + 32 + 48 * i, NULL, NULL, NULL, NULL);
    }

    return currentChoice;
}

///////////////////////
GenericSlider::GenericSlider(s32 x, s32 y, s32 l, r32 v) : xPos(x), yPos(y), length(l), value(v)
{

}
r32 GenericSlider::update()
{
    r64 xAxis = InputManager::axis(AXIS_RIGHT);

    if (xAxis > 0.0 && value <= 0.99)
        value += 0.01;
    if (xAxis < 0.0 && value >= 0.01)
        value -= 0.01;

    return value;
}

void GenericSlider::draw()
{
    //draw bg
    v4 bgColor = {0.125,0.125,0.125,1};
    Renderer::draw_rectangle(xPos, yPos, length, 16, NULL, &bgColor);
    //draw bar
    Renderer::draw_rectangle(xPos+1, yPos+1, (length * value) - 1, 14, NULL, NULL);
}

//gameplay functions
GamePlay::GamePlay(LoadingScreenType load)
{
    loadingType = load;
    //Macro::record();
    //Macro::play();
}

GamePlay::~GamePlay()
{
    //Macro::stop();
}

GameState *GamePlay::update()
{
    GameState *nextState = this;

    LevelState levelState = CurrentLevel::get_state();
    switch (levelState)
    {
    case LEVEL_PLAYING:
        //on ESC, pause game
        if (InputManager::button_down(PAUSE))
        {
            nextState = new PauseMenu(0);
            platform_pause_world_sounds();
        }
        break;
    case LEVEL_ENDED:
        nextState = new LoadingScreen(loadingType);
        break;
    default:
        break;
    }

    //update level
    CurrentLevel::update_level();
    Macro::update();

    return nextState;
}

void GamePlay::draw(r32 a)
{
    CurrentLevel::draw_level(a);
}

////////////////////////////
//gamemenu functions
MainMenu::MainMenu(u32 pos) : menu (392, 288, 240, 240, {"PLAY DEMO", "SETTINGS", "QUIT GAME"}, pos)
{
}

MainMenu::~MainMenu()
{

}

GameState *MainMenu::update()
{
    GameState *nextState = this;

    s32 menuResult = menu.update();

    if (InputManager::button_down(JUMP))
    {
        if (menuResult == 0)
        {
            //nextState = new GamePlay();
            CurrentLevel::set_fname("res/levels/dev_room.tmx");
            nextState = new LoadingScreen(LOAD_LEVEL);
        }

        if (menuResult == 1)
            nextState = new SettingsMenu({MAIN_MENU, 1});

        if (menuResult == 2)
            nextState = nullptr;
    }

    Renderer::draw_texture("tex_logo", 256, 0, NULL, NULL, NULL, NULL);

    //copyright
    Renderer::draw_string("\x81 2017 JPR", 432, 544, NULL, NULL, NULL, NULL);

    return nextState;
}

void MainMenu::draw(r32 a)
{
    Renderer::draw_ui(a);
}

/////////////////////////////////
PauseMenu::PauseMenu(u32 pos) : menu (384, 160, 272, 272, {"RESUME GAME", "RESTART LEVEL", "SETTINGS", "MAIN MENU", "QUIT GAME"}, pos)
{
}
PauseMenu::~PauseMenu()
{

}
GameState *PauseMenu::update()
{
    GameState *nextState = this;

    s32 menuResult = menu.update();

    if (InputManager::button_down(JUMP))
    {
        if (menuResult == 0)
        {
            nextState = new GamePlay(UNLOAD_LEVEL);
            platform_resume_world_sounds();
        }

        if (menuResult == 1)
        {
            //CurrentLevel::reload_level();
            CurrentLevel::end_level(true);
            platform_resume_world_sounds();
            nextState = new GamePlay(RELOAD_LEVEL);
        }

        if (menuResult == 2)
            nextState = new SettingsMenu({PAUSE_MENU, 2});

        if (menuResult == 3)
        {
            //CurrentLevel::unload_level();
            CurrentLevel::end_level(false);
            platform_resume_world_sounds();
            platform_stop_world_sounds();
            nextState = new GamePlay(UNLOAD_LEVEL);
        }


        if (menuResult == 4)
        {
            //CurrentLevel::unload_level();
            nextState = nullptr;
        }
    }

    if (InputManager::button_down(PAUSE))
    {
        nextState = new GamePlay(UNLOAD_LEVEL);
        platform_resume_world_sounds();
    }

    v4 grey = {0.5,0.5,0.5,1};
    bool32 vFlip = true;
    Renderer::draw_texture(platform_get_screenshot(), 0, 0, NULL, NULL, &vFlip, &grey, -1);

    return nextState;
}

void PauseMenu::draw(r32 a)
{
    Renderer::draw_ui(a);
}

///////////////////////////////
SettingsMenu::SettingsMenu(OptionsPrevState p) : menu (192, 176, 240, 224, {"SFX VOLUME", "MUSIC VOLUME", "FULLSCREEN", "BACK"}, 0),
                                                prevState(p),
                                                sfxSlider(528, 208, 256, Config::get_sfx_volume()),
                                                musicSlider(528, 256, 256, Config::get_music_volume())
{
    //platform_play_music(Resource::get_music("dynatest_idle.ogg"), 1);
    //platform_play_music(Resource::get_music("dynatest_drums.ogg"), 1);
}
SettingsMenu::~SettingsMenu()
{
    //platform_stop_music(1);
}
GameState *SettingsMenu::update()
{
    GameState *nextState = this;

    s32 menuResult = menu.update();

    if (menuResult == 0)
        Config::set_sfx_volume(sfxSlider.update());

    else if (menuResult == 1)
        Config::set_music_volume(musicSlider.update());

    else if (InputManager::button_down(JUMP))
    {
        if (menuResult == 2)
        {
            Config::toggle_fullscreen();
        }

        if (menuResult == 3)
        {
            nextState = go_back();
        }
    }

    if (InputManager::button_down(PAUSE))
        nextState = go_back();

    //fullscreen
    if (Config::get_fullscreen())
        Renderer::draw_string("ON", 528, 304, NULL, NULL, NULL, NULL);
    else Renderer::draw_string("OFF", 528, 304, NULL, NULL, NULL, NULL);

    sfxSlider.draw();
    musicSlider.draw();

    return nextState;
}

void SettingsMenu::draw(r32 a)
{
    Renderer::draw_ui(a);
}

GameState *SettingsMenu::go_back()
{
    Config::save_config();
    if (prevState.prevMenu == PAUSE_MENU)
        return new PauseMenu(prevState.prevPos);
    else return new MainMenu(prevState.prevPos);
}
/////////////////////////////
LevelSelect::LevelSelect()
{
}

LevelSelect::~LevelSelect()
{
}

GameState *LevelSelect::update()
{
    return this;
}
void LevelSelect::draw(r32 a)
{

}
////////////////////////////////////////
LoadingScreen::LoadingScreen(LoadingScreenType t)
{
    loadingSprite.sprite = Resource::get_sprite("spr_loading");
    done = false;
    currentTime = 0;
    minFramesVisible = 0;

    type = t;
    switch (type)
    {
    case LOAD_LEVEL:
        {
            loadingThread = new std::thread(CurrentLevel::load_level, &done);
            break;
        }
    case RELOAD_LEVEL:
        {
            CurrentLevel::clear_entities_and_tiles();
            loadingThread = new std::thread(CurrentLevel::load_level, &done);
            break;
        }
    case UNLOAD_LEVEL:
        {
            CurrentLevel::clear_entities_and_tiles();
            loadingThread = NULL;
            done = true;
            break;
        }
    default:
        break;
    }

}
LoadingScreen::~LoadingScreen()
{

}
GameState *LoadingScreen::update()
{
    GameState *nextState = this;

    loadingSprite.update(0.1);
    Renderer::draw_sprite_instance_hud(&loadingSprite, 32, 512);

    if (done && currentTime >= minFramesVisible)
    {
        if (loadingThread && loadingThread->joinable())
        {
            loadingThread->join();
            delete loadingThread;
            loadingThread = NULL;
        }

        if (done == LOADING_FAILED)
            throw std::runtime_error("Loading failed!");

        switch (type)
        {
        case LOAD_LEVEL:
        case RELOAD_LEVEL:
            {
                nextState = new GamePlay(UNLOAD_LEVEL);
                World::create_index_maps();
                break;
            }
        case UNLOAD_LEVEL:
            {
                nextState = new MainMenu(0);
                CurrentLevel::clear_entities_and_tiles();
                break;
            }
        default:
            break;
        }
    }

    currentTime++;

    return nextState;
}
void LoadingScreen::draw(r32 a)
{
    Renderer::draw_ui(a);
}
