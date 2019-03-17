#include "gamedev_game.h"

void renderFilledRect(SDL_Renderer* renderer, SDL_Rect* dest, u32 color)
{
    // COLOR_NONE is 0. Set a tile's background color to COLOR_NONE to avoid
    // extra rendering.
    if (color)
    {
        u8 r = (u8)((color & 0x00FF0000) >> 16);
        u8 g = (u8)((color & 0x0000FF00) >> 8);
        u8 b = (u8)((color & 0x000000FF) >> 0);
        SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
        SDL_RenderFillRect(renderer, dest);
    }
}

void game_destroy(Game* g)
{
    Mix_Quit();
    SDL_DestroyRenderer(g->renderer);
    SDL_DestroyWindow(g->window);
    SDL_Quit();

}

void initColors(Game* g)
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
    g->colors[COLOR_DARK_BLUE] = SDL_MapRGB(window_pixel_format, 0, 51, 102);
}

void initCamera(Game* g)
{
    g->camera.viewport.w = g->screen_width;
    g->camera.viewport.h = g->screen_height;
    g->camera.starting_pos = g->camera.viewport;

    if (g->camera.viewport.w >= g->current_map->width_pixels)
    {
        g->camera.max_x = 0;
    }
    else
    {
        g->camera.max_x = g->current_map->width_pixels - g->camera.viewport.w;
    }
    if (g->camera.viewport.h >= g->current_map->width_pixels)
    {
        g->camera.max_y = 0;
    }
    else
    {
        g->camera.max_y = absInt32(g->current_map->height_pixels - g->camera.viewport.h);
    }
    g->camera.y_pixel_movement_threshold = g->screen_height / 2;
    g->camera.x_pixel_movement_threshold = g->screen_width / 2;
}

void initGame(Game* g, u32 width, u32 height)
{
    g->screen_width = width;
    g->screen_height = height;
    g->target_fps = 60;
    g->dt = (i32)((1.0f / (f32)g->target_fps) * 1000);
    g->target_ms_per_frame = (u32)(1000.0f / (f32)g->target_fps);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
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
    initColors(g);
    g->initialized = GD_TRUE;
    g->running = GD_TRUE;
}

void updateGame(Game *g, Input *input)
{
    if (input->key_pressed[KEY_ESCAPE])
    {
        g->running = GD_FALSE;
    }
}

void startDialogMode(Game *g, char *dialog)
{
    g->mode = GAME_MODE_DIALOG;
    g->dialog = dialog;
}

void endDialogMode(Game *g)
{
    g->mode = GAME_MODE_PLAYING;
    g->dialog = NULL;
}

void updateDialogMode(Game *g, Input *input)
{
    if (input->key_pressed[KEY_ESCAPE])
    {
        endDialogMode(g);
    }
}

void startInventoryMode(Game *g)
{
    g->mode = GAME_MODE_INVENTORY;
}

void endInventoryMode(Game *g)
{
    g->mode = GAME_MODE_PLAYING;
}

void updateInventoryMode(Game *g, Input *input)
{
    if (input->key_pressed[KEY_ESCAPE])
    {
        endInventoryMode(g);
    }
}

void drawDialogScreen(Game *g, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogBoxX = (int)(0.5 * (thirdOfWidth)) + g->camera.viewport.x;
    int dialogBoxY = (int)((3 * (fourthOfHeight)) - 0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogBoxWidth = 2 * (thirdOfWidth);
    int dialogBoxHeight = fourthOfHeight;
    SDL_Rect dialogBoxDest = {dialogBoxX,dialogBoxY, dialogBoxWidth, dialogBoxHeight};
    renderFilledRect(g->renderer, &dialogBoxDest, g->colors[COLOR_DARK_BLUE]);
    drawText(g, fontMetadata, g->dialog, dialogBoxX, dialogBoxY);
}

void drawInventoryScreen(Game *g, Hero *h, FontMetadata *fontMetadata)
{
    int thirdOfWidth = (int)(g->camera.viewport.w / 3);
    int fourthOfHeight = (int)(g->camera.viewport.h / 4);
    int dialogBoxX = (int)(0.5 * thirdOfWidth) + g->camera.viewport.x;
    int dialogBoxY = (int)(0.5 * fourthOfHeight) + g->camera.viewport.y;
    int dialogBoxWidth = 2 * (thirdOfWidth);
    int dialogBoxHeight = fourthOfHeight;
    SDL_Rect dialogBoxDest = {dialogBoxX,dialogBoxY, dialogBoxWidth, dialogBoxHeight};
    renderFilledRect(g->renderer, &dialogBoxDest, g->colors[COLOR_DARK_BLUE]);
    char leaves[30];
    snprintf(leaves, 30, "Leaves: %d", h->inventory[INV_LEAVES]);
    drawText(g, fontMetadata, leaves, dialogBoxX, dialogBoxY);
}

void darkenBackground(Game *g)
{
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 32);
    SDL_RenderFillRect(g->renderer, NULL);
}

void drawDebugCircle(SDL_Renderer *renderer, i32 _x, i32 _y, i32 radius)
{
    i32 x = radius - 1;
    if (radius == 0)
        return;
    i32 y = 0;
    i32 tx = 1;
    i32 ty = 1;
    i32 err = tx - (radius << 1); // shifting bits left by 1 effectively
    // doubles the value. == tx - diameter
    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, _x + x, _y - y);
        SDL_RenderDrawPoint(renderer, _x + x, _y + y);
        SDL_RenderDrawPoint(renderer, _x - x, _y - y);
        SDL_RenderDrawPoint(renderer, _x - x, _y + y);
        SDL_RenderDrawPoint(renderer, _x + y, _y - x);
        SDL_RenderDrawPoint(renderer, _x + y, _y + x);
        SDL_RenderDrawPoint(renderer, _x - y, _y - x);
        SDL_RenderDrawPoint(renderer, _x - y, _y + x);

        if (err <= 0)
        {
            y++;
            err += ty;
            ty += 2;
        }
        if (err > 0)
        {
            x--;
            tx += 2;
            err += tx - (radius << 1);
        }
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
