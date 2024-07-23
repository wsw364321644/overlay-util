#pragma once
#include <SDL2/SDL_video.h>
#include "mouse_event.h"

typedef struct window_resize_event_t
{
    uint16_t width;
    uint16_t height;
} window_resize_event_t;

typedef struct window_move_event_t
{
    uint16_t x;
    uint16_t y;
} window_move_event_t;

typedef struct window_event_t
{
    SDL_WindowEventID event:8;
    union
    {
        mouse_motion_event_t mouse_motion;
        window_resize_event_t win_size;
        window_move_event_t win_move;
    }data;
} window_event_t;
