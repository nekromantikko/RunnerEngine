#include "input.h"
#include "macro.h"

namespace InputManager
{
    Input input;
    MacroInputProcessed macroInput;
}

const r64 InputManager::axis(InputAxis axis)
{
    r64 axisSum = 0.f;
    s32 inputAmount = 4;

    switch(axis)
    {
    case AXIS_RIGHT:
        {
            if (input.controller.xAxisLeft == OFF)
                inputAmount--;
            else axisSum += input.controller.xAxisLeft.get_state();

            if (input.keyboard.keyLeft == OFF)
                inputAmount--;
            else axisSum -= 1;

            if (input.keyboard.keyRight == OFF)
                inputAmount--;
            else axisSum += 1;

            if (macroInput.xAxis == OFF)
                inputAmount--;
            else axisSum += macroInput.xAxis.get_state();
        }
        break;
    case AXIS_UP:
        {
            if (input.controller.yAxisLeft == OFF)
                inputAmount--;
            else axisSum -= input.controller.yAxisLeft.get_state();

            if (input.keyboard.keyDown == OFF)
                inputAmount--;
            else axisSum -= 1;

            if (input.keyboard.keyUp == OFF)
                inputAmount--;
            else axisSum += 1;

            if (macroInput.yAxis == OFF)
                inputAmount--;
            else axisSum += macroInput.yAxis.get_state();
        }
        break;
    default:
        break;
    }

    if (inputAmount != 0)
        return axisSum / inputAmount;
    else return 0.0;
}

const bool InputManager::axis_moved(InputAxis axis)
{
    bool result = false;

    switch(axis)
    {
    case AXIS_RIGHT:
        {
            result = (result || input.keyboard.keyLeft == BTN_JUST_PRESSED);
            result = (result || input.keyboard.keyRight == BTN_JUST_PRESSED);
            result = (result || input.controller.xAxisLeft == BTN_JUST_PRESSED);
            result = (result || macroInput.xAxis == BTN_JUST_PRESSED);
        }
        break;
    case AXIS_UP:
        {
            result = (result || input.keyboard.keyUp == BTN_JUST_PRESSED);
            result = (result || input.keyboard.keyDown == BTN_JUST_PRESSED);
            result = (result || input.controller.yAxisLeft == BTN_JUST_PRESSED);
            result = (result || macroInput.yAxis == BTN_JUST_PRESSED);
        }
        break;
    default:
        break;
    }

    return result;
}

const bool InputManager::axis_released(InputAxis axis)
{
    bool result = false;

    switch(axis)
    {
    case AXIS_RIGHT:
        {
            result = (result || input.keyboard.keyLeft == BTN_JUST_RELEASED);
            result = (result || input.keyboard.keyRight == BTN_JUST_RELEASED);
            result = (result || input.controller.xAxisLeft == BTN_JUST_RELEASED);
            result = (result || macroInput.xAxis == BTN_JUST_RELEASED);
        }
        break;
    case AXIS_UP:
        {
            result = (result || input.keyboard.keyUp == BTN_JUST_RELEASED);
            result = (result || input.keyboard.keyDown == BTN_JUST_RELEASED);
            result = (result || input.controller.yAxisLeft == BTN_JUST_RELEASED);
            result = (result || macroInput.yAxis == BTN_JUST_RELEASED);
        }
        break;
    default:
        break;
    }

    return result;
}

const bool InputManager::button(InputAction action)
{
    bool result = false;

    switch(action)
    {
    case JUMP:
        {
            result = (result || input.keyboard.keySpace == ON);
            result = (result || input.controller.aButton == ON);
            result = (result || macroInput.jump == ON);
        }
        break;
    case DASH:
        {
            result = (result || input.keyboard.keyLeftShift == ON);
            result = (result || input.controller.triggerAxisRight == ON);
            result = (result || macroInput.dash == ON);
        }
        break;
    case PAUSE:
        {
            result = (result || input.keyboard.keyEscape == ON);
            result = (result || input.controller.startButton == ON);
        }
        break;
    case SHOOT:
        {
            result = (result || input.keyboard.keyD == ON);
            result = (result || input.controller.xButton == ON);
        }
        break;
    default:
        break;
    }

    return result;
}

const bool InputManager::button_down(InputAction action)
{
    bool result = false;

    switch(action)
    {
    case JUMP:
        {
            result = (result || input.keyboard.keySpace == BTN_JUST_PRESSED);
            result = (result || input.controller.aButton == BTN_JUST_PRESSED);
            result = (result || macroInput.jump == BTN_JUST_PRESSED);
        }
        break;
    case DASH:
        {
            result = (result || input.keyboard.keyLeftShift == BTN_JUST_PRESSED);
            result = (result || input.controller.triggerAxisRight == BTN_JUST_PRESSED);
            result = (result || macroInput.dash == BTN_JUST_PRESSED);
        }
        break;
    case PAUSE:
        {
            result = (result || input.keyboard.keyEscape == BTN_JUST_PRESSED);
            result = (result || input.controller.startButton == BTN_JUST_PRESSED);
        }
        break;
    case SHOOT:
        {
            result = (result || input.keyboard.keyD == BTN_JUST_PRESSED);
            result = (result || input.controller.xButton == BTN_JUST_PRESSED);
        }
        break;
    default:
        break;
    }

    return result;
}

const bool InputManager::button_up(InputAction action)
{
    bool result = false;

    switch(action)
    {
    case JUMP:
        {
            result = (result || input.keyboard.keySpace == BTN_JUST_RELEASED);
            result = (result || input.controller.aButton == BTN_JUST_RELEASED);
            result = (result || macroInput.jump == BTN_JUST_RELEASED);
        }
        break;
    case DASH:
        {
            result = (result || input.keyboard.keyLeftShift == BTN_JUST_RELEASED);
            result = (result || input.controller.triggerAxisRight == BTN_JUST_RELEASED);
            result = (result || macroInput.dash == BTN_JUST_RELEASED);
        }
        break;
    case PAUSE:
        {
            result = (result || input.keyboard.keyEscape == BTN_JUST_RELEASED);
            result = (result || input.controller.startButton == BTN_JUST_RELEASED);
        }
        break;
    case SHOOT:
        {
            result = (result || input.keyboard.keyD == BTN_JUST_RELEASED);
            result = (result || input.controller.xButton == BTN_JUST_RELEASED);
        }
        break;
    default:
        break;
    }

    return result;
}

Input *InputManager::get_input()
{
    return &input;
}

void InputManager::refresh()
{
    input.keyboard.keyDown.refresh();
    input.keyboard.keyUp.refresh();
    input.keyboard.keyRight.refresh();
    input.keyboard.keyR.refresh();
    input.keyboard.keyF1.refresh();
    input.keyboard.keyLeft.refresh();
    input.keyboard.keyLeftShift.refresh();
    input.keyboard.keySpace.refresh();
    input.keyboard.keyEscape.refresh();
    input.keyboard.keyD.refresh();

    input.controller.aButton.refresh();
    input.controller.bButton.refresh();
    input.controller.startButton.refresh();
    input.controller.triggerAxisRight.refresh();
    input.controller.xAxisLeft.refresh();
    input.controller.xButton.refresh();
    input.controller.yAxisLeft.refresh();
    input.controller.yButton.refresh();

    MacroInput macro;
    macro.horState = axis(AXIS_RIGHT);
    macro.verState = axis(AXIS_UP);
    macro.jumpState = button(JUMP);
    macro.dashState = button(DASH);

    Macro::add_virtual_input(macro);
}

void InputManager::set_macro_input(MacroInputProcessed input)
{
    macroInput = input;
}
