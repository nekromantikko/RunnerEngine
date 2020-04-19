#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include "typedef.h"
#include "sprite.h"

class Level;

class GameState
{
public:
    virtual GameState *update() = 0;
    virtual void draw(r32) = 0;
    virtual ~GameState() {}
};

class GenericMenu
{
public:
    GenericMenu(s32 x, s32 y, s32 w, s32 h, std::vector<const char*> i, u32 pos);
    u8 update();
private:
    s32 xPos, yPos, width, height;
    std::vector<const char*> items;
    u8 currentChoice = 0;
    bool buttonDown = false;
};

class GenericSlider
{
public:
    GenericSlider(s32 x, s32 y, s32 l, r32 v = 0.0);
    r32 update();
    void draw();
private:
    s32 xPos, yPos, length;
    r32 value;
};
//////////////
enum LoadingScreenType
{
    LOAD_LEVEL,
    RELOAD_LEVEL,
    UNLOAD_LEVEL
};

//state that plays the levels
class GamePlay : public GameState
{
public:
    GamePlay(LoadingScreenType load);
    ~GamePlay();
    GameState *update();
    void draw(r32);
private:
    bool fullscreen = false;
    //the type of loading to do when the level ends
    LoadingScreenType loadingType;
};
//////////////////////////////
class MainMenu : public GameState
{
public:
    MainMenu(u32 pos);
    ~MainMenu();
    GameState *update();
    void draw(r32);
private:
    GenericMenu menu;
};
///////////////////////////////////////
class PauseMenu : public GameState
{
public:
    PauseMenu(u32 pos);
    ~PauseMenu();
    GameState *update();
    void draw(r32);
private:
    GenericMenu menu;
};
//////////////////////////////////////////
enum OptionsPrevMenu : bool
{
    MAIN_MENU = false,
    PAUSE_MENU = true
};

struct OptionsPrevState
{
    OptionsPrevMenu prevMenu;
    u32 prevPos;
};


class SettingsMenu : public GameState
{
public:
    SettingsMenu(OptionsPrevState);
    ~SettingsMenu();
    GameState *update();
    void draw(r32);
private:
    inline GameState *go_back();
    GenericMenu menu;
    OptionsPrevState prevState;
    //sliders
    GenericSlider sfxSlider, musicSlider;
};
///////////////////////////////
class LevelSelect : public GameState
{
public:
    LevelSelect();
    ~LevelSelect();
    GameState *update();
    void draw(r32);
private:
};

enum LoadingStatus
{
    LOADING_FAILED = -1,
    LOADING_IN_PROGRESS = 0,
    LOADING_COMPLETE = 1
};

class LoadingScreen : public GameState
{
public:
    LoadingScreen(LoadingScreenType t);
    ~LoadingScreen();
    GameState *update();
    void draw(r32);
private:
    LoadingScreenType type;
    SpriteInstance loadingSprite;
    std::atomic<s32> done;
    std::thread *loadingThread;
    u32 minFramesVisible;
    u32 currentTime;
};

#endif // GAMESTATE_H
