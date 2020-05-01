#include "../platform_main.h"
#include "../platform_rendering.h"
#include "../platform_audio.h"
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

//THIS IS THE PLATFORM CODE FOR WINDOWS!
//MAC + LINUX WOULD BE ALMOST THE SAME, EXCEPT WITH DIFFERENT VERSION OF FMOD

SDL_GLContext runnerGlContext;
SDL_Window *runnerWindow;

//INIT / QUIT

void init_SDL()
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_HAPTIC);

    //Use openGL 4.3 (for now)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
}

void platform_init()
{
    //initialize sdl
    init_SDL();

    //create window and opengl context
    runnerWindow = SDL_CreateWindow("Parcool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    runnerGlContext = SDL_GL_CreateContext(runnerWindow);

    //initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
    //set clear color to black
    glClearColor(0.f, 0.f, 0.f, 0.f);

    platform_init_rendering();

    platform_init_audio();
}

void platform_quit()
{
    platform_deinit_rendering();

    //destroy opengl context and window
    SDL_GL_DeleteContext(runnerGlContext);
    SDL_DestroyWindow(runnerWindow);

    //quit sdl
    SDL_Quit();

    platform_deinit_audio();
}

void platform_show_error(const char* error)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error!", error, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////

r64 platform_time_in_ms()
{
    return SDL_GetPerformanceCounter() / (SDL_GetPerformanceFrequency() / 1000.0);
}
u64 platform_get_ticks()
{
    return SDL_GetPerformanceCounter();
}
void platform_disable_vsync()
{
    SDL_GL_SetSwapInterval(0);
}
void platform_enable_vsync()
{
    SDL_GL_SetSwapInterval(1);
}
u64 platform_ms_to_ticks(r64 t)
{
    r64 timeInSeconds = t / 1000.f;
    return std::round(timeInSeconds * SDL_GetPerformanceFrequency());
}
r64 platform_ticks_to_ms(u64 t)
{
    r64 ticksPerSecond = t * 1000;
    return ticksPerSecond / SDL_GetPerformanceFrequency();
}

///////////////////////////////////////////////////////////////////////////////////////////

//INPUT
SDL_GameController *gameController = NULL;
SDL_Haptic *haptic = NULL;

void platform_init_controller()
{
    if (SDL_NumJoysticks())
    {
        gameController = SDL_GameControllerOpen(0);
        haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gameController));
        SDL_HapticRumbleInit(haptic);
    }
    else gameController = NULL;
}

void platform_deinit_controller()
{
    if (gameController)
        SDL_GameControllerClose(gameController);
    if (haptic)
    {
        SDL_HapticRumbleStop(haptic);
        SDL_HapticClose(haptic);
    }
}

bool platform_controller_exists()
{
    return (gameController != 0);
}

void handle_keyboard_event(SDL_KeyboardEvent &event, Input *input)
{
    switch(event.keysym.scancode)
    {
    case SDL_SCANCODE_LEFT:
        input->keyboard.keyLeft = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_RIGHT:
        input->keyboard.keyRight = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_UP:
        input->keyboard.keyUp = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_DOWN:
        input->keyboard.keyDown = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_SPACE:
        input->keyboard.keySpace = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_LSHIFT:
        input->keyboard.keyLeftShift = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_ESCAPE:
        input->keyboard.keyEscape = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_F1:
        input->keyboard.keyF1 = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_R:
        input->keyboard.keyR = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_D:
        input->keyboard.keyD = (event.state == SDL_PRESSED);
        break;
    default:
        break;
    }
}

void handle_controller_button_event(SDL_ControllerButtonEvent &event, Input *input)
{
    switch(event.button)
    {
    case SDL_CONTROLLER_BUTTON_A:
        input->controller.aButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_B:
        input->controller.bButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_X:
        input->controller.xButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        input->controller.yButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_START:
        input->controller.startButton = (event.state == SDL_PRESSED);
        break;
    default:
        break;
    }
}

void handle_controller_axis_event(SDL_ControllerAxisEvent &event, Input *input)
{
    switch(event.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        input->controller.xAxisLeft = event.value / 32767.0;
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        input->controller.yAxisLeft = event.value / 32767.0;
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        input->controller.triggerAxisRight = event.value / 32767.0;
        break;
    default:
        break;
    }
}

void handle_controller_device_event(SDL_ControllerDeviceEvent &event, Input *input)
{
    switch(event.type)
    {
    case SDL_CONTROLLERDEVICEADDED:
        if (event.which == 0)
        {
            if (!gameController)
                platform_init_controller();
        }
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        if (event.which == 0)
        {
            if (gameController)
                platform_deinit_controller();
        }

        break;
    default:
        break;
    }
}


void platform_poll_input(Input *input)
{
    bool controllerExists = platform_controller_exists();
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            input->exit = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handle_keyboard_event(event.key, input);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (controllerExists)
                handle_controller_button_event(event.cbutton, input);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (controllerExists)
                handle_controller_axis_event(event.caxis, input);
            break;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            handle_controller_device_event(event.cdevice, input);
        default:
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////

//haptic
void platform_controller_rumble(r32 strength, u32 lengthInMs)
{
    if (haptic)
    {
        SDL_HapticRumblePlay(haptic, strength, lengthInMs);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

void platform_set_fullscreen(bool s)
{
    if (s)
    {
        SDL_ShowCursor(false);
        SDL_SetWindowFullscreen(runnerWindow, SDL_WINDOW_FULLSCREEN);
    }
    else
    {
        SDL_ShowCursor(true);
        SDL_SetWindowFullscreen(runnerWindow, 0);
    }
}

void platform_swap_buffer()
{
    SDL_GL_SwapWindow(runnerWindow);
}

