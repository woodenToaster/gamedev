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
    COLOR_COUNT
};

enum GameMode
{
    GAME_MODE_PLAYING,
    GAME_MODE_DIALOGUE,
    GAME_MODE_INVENTORY,
    GAME_MODE_COUNT
};

struct GameMemory
{
    u64 permanentStorageSize;
    void *permanentStorage;

    u64 transientStorageSize;
    void *transientStorage;
};

struct Game
{
    u32 dt;
    i32 screenWidth;
    i32 screenHeight;
    i32 targetFps;
    u32 totalFramesElapsed;
    u32 targetMsPerFrame;
    bool running;
    u8 initialized;
    SDL_Window *window;
    SDL_Surface *windowSurface;
    SDL_Renderer *renderer;
    u32 colors[COLOR_COUNT];
    Camera camera;
    Map *currentMap;
    SoundList sounds;
    GameMode mode;
    char *dialogue;

    SDL_Texture *linkTexture;
    SDL_Texture *treeTexture;
    SDL_Texture *treeStumpTexture;
    SDL_Texture *harlodTexture;
    SDL_Texture *knightTexture;
    SDL_Texture *fireTileTexture;

    Sound mudSound;
    // TODO(chj): Replace SoundList
    // u32 soundCount;
    // Sound *sounds[16];
};

void startDialogueMode(Game *g, char *dialogue);
void endDialogueMode(Game *g);
void startInventoryMode(Game *g);
void endInventoryMode(Game *g);
#endif
