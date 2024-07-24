#pragma once
#include <cstdint>

typedef struct overlay_ime_event_t
{
    bool    want_visible{ false };
    uint16_t  input_pos_x{ 0 };
    uint16_t  input_pos_y{ 0 };
    uint16_t  input_line_height{ 0 };
}overlay_ime_event_t;

typedef struct overlay_char_event_t
{
    uint16_t num;
    const char* char_buf;
}overlay_char_event_t;