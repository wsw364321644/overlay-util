#pragma once
#include <simdjson.h>
#include <rapidjson/document.h>
typedef rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>, rapidjson::MemoryPoolAllocator<>> DocumentType;
extern thread_local char ValueBuffer[4096];
extern thread_local char ParseBuffer[1024];
extern thread_local rapidjson::MemoryPoolAllocator<> ThreadValueAllocator;
extern thread_local rapidjson::MemoryPoolAllocator<> ThreadParseAllocator;
extern thread_local simdjson::ondemand::parser SimdjsonParser;