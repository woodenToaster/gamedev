#ifndef GD_GAME_H
#define GD_GAME_H

#include "gamedev_tilemap.h"

enum Colors
{
    GREEN,
    BLUE,
    YELLOW,
    BROWN,
    RUST,
    MAGENTA,
    BLACK,
    RED,
    GREY,
    COLOR_LAST
};

struct Game
{
    int screen_width;
    int screen_height;
    u32 frames;
    bool running;
    u8 initialized;
    SDL_Window* window;
    SDL_Surface* window_surface;
    u32 colors[COLOR_LAST];
    Camera camera;
    Map* current_map;
};
#endif
