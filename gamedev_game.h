#ifndef GD_GAME_H
#define GD_GAME_H

struct Game
{
    int screen_width;
    int screen_height;
    u32 frames;
    bool running;
    u8 initialized;
    SDL_Window* window;
    SDL_Surface* window_surface;
};
#endif
