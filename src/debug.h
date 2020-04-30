#ifndef DEBUG_H
#define DEBUG_H
#include "typedef.h"
#include "Platform/platform_main.h"

#define TIMED_BLOCK TimedBlock timedBlock##__LINE__(__COUNTER__, (char*)__FILE__, __LINE__, (char*)__FUNCTION__);

struct DebugRecord
{
    u64 timestamp;
    char *fileName;
    char *functionName;
    u32 lineNumber;
};

namespace Debug
{
    void add_debug_record(char *file, u32 counter, DebugRecord record);
    DebugRecord *get_debug_record(char* file, u32 counter);
    void print_records();
}

struct TimedBlock
{
    DebugRecord *record;

    TimedBlock(u32 counter, char *fname, int lineNumber, char *functionName)
    {
        record = Debug::get_debug_record(fname, counter);
        record->fileName = fname;
        record->functionName = functionName;
        record->lineNumber = lineNumber;
        record->timestamp = -platform_get_ticks();
    }

    ~TimedBlock()
    {
        record->timestamp += platform_get_ticks();
    }
};

#endif // DEBUG_H
