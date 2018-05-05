#include "gamedev_game.h"

void game_destroy(Game* g)
{
    Mix_Quit();
    SDL_DestroyWindow(g->window);
    SDL_Quit();
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
    g->initialized = GD_TRUE;
}
