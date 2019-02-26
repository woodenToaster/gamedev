#include "gamedev_game.h"

void game_destroy(Game* g)
{
    Mix_Quit();
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

void game_init_colors(Game* g)
{
    SDL_PixelFormat* window_pixel_format = g->window_surface->format;
    g->colors[COLOR_NONE] = 0;
    g->colors[COLOR_GREEN] = SDL_MapRGB(window_pixel_format, 37, 71, 0);
    g->colors[COLOR_BLUE] = SDL_MapRGB(window_pixel_format, 0, 0, 255);
    g->colors[COLOR_YELLOW] = SDL_MapRGB(window_pixel_format, 235, 245, 65);
    g->colors[COLOR_BROWN] = SDL_MapRGB(window_pixel_format, 153, 102, 0);
    g->colors[COLOR_RUST] = SDL_MapRGB(window_pixel_format, 153, 70, 77);
    g->colors[COLOR_MAGENTA] = SDL_MapRGB(window_pixel_format, 255, 0, 255);
    g->colors[COLOR_BLACK] = SDL_MapRGB(window_pixel_format, 0, 0, 0);
    g->colors[COLOR_RED] = SDL_MapRGB(window_pixel_format, 255, 0, 0);
    g->colors[COLOR_GREY] = SDL_MapRGB(window_pixel_format, 135, 135, 135);
}

void game_init_camera(Game* g)
{
    g->camera.viewport.w = g->screen_width;
    g->camera.viewport.h = g->screen_height;
    g->camera.starting_pos = g->camera.viewport;
    g->camera.max_x = g->current_map->width_pixels - g->camera.viewport.w;
    g->camera.max_y = g->current_map->height_pixels - g->camera.viewport.h;
    g->camera.y_pixel_movement_threshold = g->screen_height / 2;
    g->camera.x_pixel_movement_threshold = g->screen_width / 2;
}

void game_init(Game* g, u32 width, u32 height)
{
    g->screen_width = width;
    g->screen_height = height;
    g->target_fps = 60;
    g->dt = 16;
    g->target_ms_per_frame = (u32)(1000.0f / (f32)g->target_fps);

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
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (g->window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    g->window_surface = SDL_GetWindowSurface(g->window);

    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED);

    if (g->renderer == NULL)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    game_init_colors(g);
    g->initialized = GD_TRUE;
    g->running = GD_TRUE;
}

void game_update(Game* g, Input* input)
{
    if (input->is_pressed[KEY_ESCAPE])
    {
        g->running = GD_FALSE;
    }
}

void game_fix_frame_rate(Game* g)
{
    if (g->dt < g->target_ms_per_frame)
    {
        while (g->dt < g->target_ms_per_frame)
        {
            u32 sleep_ms = g->target_ms_per_frame - g->dt;
            g->dt += sleep_ms;
            SDL_Delay(sleep_ms);
        }
    }
    else
    {
#ifdef DEBUG
        printf("Frame missed!\n");
#endif
    }

    g->total_frames_elapsed++;
}
