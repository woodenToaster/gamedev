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
    COLOR_DARK_BLUE,
    COLOR_BABY_BLUE,
    COLOR_DARK_ORANGE,
    COLOR_LAST
};

enum GameMode
{
    GAME_MODE_PLAYING,
    GAME_MODE_DIALOG,
    GAME_MODE_INVENTORY,
    GAME_MODE_COUNT
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
    SDL_Window *window;
    SDL_Surface *window_surface;
    SDL_Renderer *renderer;
    u32 colors[COLOR_LAST];
    Camera camera;
    Map *current_map;
    // MapList *maps;
    SoundList sounds;
    GameMode mode;
    char *dialog;

    SDL_Texture *linkTexture;
    SDL_Texture *treeTexture;
    SDL_Texture *treeStumpTexture;
    SDL_Texture *harlodTexture;
    SDL_Texture *knightTexture;

    Sound mudSound;
    u32 soundCount;
};

void startDialogMode(Game *g, char *dialog);
void endDialogMode(Game *g);
void startInventoryMode(Game *g);
void endInventoryMode(Game *g);
#endif
