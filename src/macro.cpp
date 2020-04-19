#include "macro.h"
#include "file_formats.h"
#include <iostream>
#include "compression.h"
#include <utility>
#include <fstream>
#include "input.h"

namespace Macro
{
    MacroState state;
    std::vector<MacroInput> recInputs;
    std::vector<MacroInputProcessed> playInputs;
    u64 currentFrame;
}

void Macro::init()
{
    recInputs.reserve(MACRO_MAX_LENGTH);

    state = MACRO_STOPPED;
    currentFrame = 0;
}
void Macro::deinit()
{

}
void Macro::save_current_macro(const char *fname)
{
    InputMacroHeader header;
    header.signature = MACRO_SIGNATURE;
    header.version = MACRO_VERSION;
    header.frameCount = recInputs.size();

    std::vector<u8> fileBody;

    for (int i = 0; i < header.frameCount; i++)
    {
        fileBody.insert(fileBody.end(), (u8*)&recInputs.at(i), ((u8*)&recInputs.at(i)) + sizeof(MacroInput));
    }

    std::ofstream outFile(fname, std::ios::binary);

    outFile.write((const char*)&header, sizeof(InputMacroHeader));
    outFile.write((const char*)fileBody.data(), fileBody.size());

    outFile.close();
    std::cout << "MACRO SAVED!\n";

}
bool Macro::load_playback_macro(const char *fname)
{
    bool result = false;

    std::ifstream file(fname, std::ios::binary);

    if (file)
    {
        InputMacroHeader header;
        file.read((char*)&header, sizeof(header));

        if (header.signature != MACRO_SIGNATURE)
            throw std::runtime_error("Macro is not a proper rmac-file!");
        else if (header.version != MACRO_VERSION)
            throw std::runtime_error("Macro file is not the right version!");
        else
        {
            MacroInput input;
            for (int i = 0; i < header.frameCount; i++)
            {
                MacroInputProcessed processedInput;
                if (i == 0)
                {
                    processedInput.xAxis.prevState_ = 0;
                    processedInput.yAxis.prevState_ = 0;
                    processedInput.jump.prevState_ = 0;
                    processedInput.dash.prevState_ = 0;
                }
                else
                {
                    processedInput.xAxis.prevState_ = input.horState;
                    processedInput.yAxis.prevState_ = input.verState;
                    processedInput.jump.prevState_ = input.jumpState;
                    processedInput.dash.prevState_ = input.dashState;
                }

                file.read((char*)&input, sizeof(MacroInput));

                processedInput.xAxis.state_ = input.horState;
                processedInput.yAxis.state_ = input.verState;
                processedInput.jump.state_ = input.jumpState;
                processedInput.dash.state_ = input.dashState;

                playInputs.push_back(processedInput);
            }
            result = true;
        }
        file.close();
    }
}
void Macro::add_virtual_input(MacroInput input)
{
    if (state != MACRO_RECORDING)
        return;
    recInputs.push_back(input);
}
void Macro::update()
{
    if (state == MACRO_PLAYING && currentFrame < playInputs.size())
    {
        InputManager::set_macro_input(playInputs.at(currentFrame));
        currentFrame++;
    }
}
MacroState Macro::get_current_state()
{
    return state;
}
void Macro::play()
{
    state = MACRO_PLAYING;
}
void Macro::record()
{
    state = MACRO_RECORDING;
}
void Macro::stop()
{
    state = MACRO_STOPPED;
}
