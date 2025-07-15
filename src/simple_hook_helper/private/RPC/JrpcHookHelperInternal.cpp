#include "JrpcHookHelperInternal.h"

thread_local char ValueBuffer[4096];
thread_local char ParseBuffer[1024];
thread_local rapidjson::MemoryPoolAllocator<> ThreadValueAllocator(ValueBuffer, sizeof(ValueBuffer));
thread_local rapidjson::MemoryPoolAllocator<> ThreadParseAllocator(ParseBuffer, sizeof(ParseBuffer));
thread_local simdjson::ondemand::parser SimdjsonParser;