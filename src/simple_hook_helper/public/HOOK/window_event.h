#pragma once
#include <SDL2/SDL_video.h>
#include "mouse_event.h"

typedef struct window_event_t
{
    SDL_WindowEventID event:8;
    union
    {
        mouse_motion_event_t pos;
    }data;
} window_event_t;
