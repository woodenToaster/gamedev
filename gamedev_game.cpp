#include "gamedev_game.h"

void game_destroy(Game* g)
{
    Mix_Quit();
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

void game_init_colors(Game* g)
{
    SDL_PixelFormat* window_pixel_format = g->window_surface->format;
    g->colors[GREEN] = SDL_MapRGB(window_pixel_format, 37, 71, 0);
    g->colors[BLUE] = SDL_MapRGB(window_pixel_format, 0, 0, 255);
    g->colors[YELLOW] = SDL_MapRGB(window_pixel_format, 235, 245, 65);
    g->colors[BROWN] = SDL_MapRGB(window_pixel_format, 153, 102, 0);
    g->colors[RUST] = SDL_MapRGB(window_pixel_format, 153, 70, 77);
    g->colors[MAGENTA] = SDL_MapRGB(window_pixel_format, 255, 0, 255);
    g->colors[BLACK] = SDL_MapRGB(window_pixel_format, 0, 0, 0);
    g->colors[RED] = SDL_MapRGB(window_pixel_format, 255, 0, 0);
    g->colors[GREY] = SDL_MapRGB(window_pixel_format, 135, 135, 135);
}

void game_init(Game* g, u32 width, u32 height)
{
    g->screen_width = width;
    g->screen_height = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL failed to initialize: %s\n", SDL_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        exit(1);
    }

    g->window = SDL_CreateWindow("gamedev",
                                 30,
                                 50,
                                 g->screen_width,
                                 g->screen_height,
                                 SDL_WINDOW_SHOWN);

    if (g->window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    g->window_surface = SDL_GetWindowSurface(g->window);
    game_init_colors(g);
    g->initialized = GD_TRUE;
}
