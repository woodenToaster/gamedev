#ifndef GD_GAME_H
#define GD_GAME_H

#include "gamedev_tilemap.h"

enum Colors
{
    COLOR_NONE,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_BROWN,
    COLOR_RUST,
    COLOR_MAGENTA,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREY,
    COLOR_LAST
};

struct Game
{
    u32 dt;
    i32 screen_width;
    i32 screen_height;
    i32 target_fps;
    u32 total_frames_elapsed;
    u32 target_ms_per_frame;
    bool running;
    u8 initialized;
    SDL_Window* window;
    SDL_Surface* window_surface;
    SDL_Renderer* renderer;
    u32 colors[COLOR_LAST];
    Camera camera;
    Map* current_map;
    MapList* maps;
    SoundList* sounds;
};
#endif
