#include "debug.h"
#include <map>
#include "renderer.h"
#include <sstream>

namespace Debug
{
    //key for map map is filename
    std::map<char*, std::map<u32, DebugRecord>> debugRecordMap;
}

void Debug::add_debug_record(char *file, u32 counter, DebugRecord record)
{
    debugRecordMap[file][counter] = record;
}

DebugRecord *Debug::get_debug_record(char* file, u32 counter)
{
    return &debugRecordMap[file][counter];
}

void Debug::print_records()
{
    memory_index i = 0;
    for (std::pair<char* const, std::map<u32, DebugRecord>> &recordMap : debugRecordMap)
    {
        for (std::pair<const u32, DebugRecord> &recordPair : recordMap.second)
        {
            DebugRecord &record = recordPair.second;
            std::stringstream ss;
            ss <</* "FILE: " << record.fileName <<*/ " FUNCTION: " << record.functionName << " LINE: " << record.lineNumber << " CYCLES: " << record.timestamp;
            v4 green = {0,1,0,1};
            r32 scale = 1;
            Renderer::draw_string(ss.str(), 0, 32 + i*16, NULL, &green, &scale, NULL);
            i++;
        }
    }
}
