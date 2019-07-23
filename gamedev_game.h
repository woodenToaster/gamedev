#ifndef GD_GAME_H
#define GD_GAME_H

#include "gamedev_tilemap.h"

enum Colors
{
    Color_None,
    Color_White,
    Color_DarkGreen,
    Color_Blue,
    Color_Yellow,
    Color_Brown,
    Color_Rust,
    Color_Magenta,
    Color_Black,
    Color_Red,
    Color_Grey,
    Color_DarkBlue,
    Color_BabyBlue,
    Color_DarkOrange,
    Color_LimeGreen,
    Color_Count
};

enum GameMode
{
    GameMode_Playing,
    GameMode_Dialogue,
    GameMode_Inventory,
    GameMode_Count
};

struct GameMemory
{
    u64 permanentStorageSize;
    void *permanentStorage;

    u64 transientStorageSize;
    void *transientStorage;

    PlatformAPI platformAPI;
};

struct Arena
{
    size_t used;
    size_t maxCap;
    i32 tmpCount;
    u8 *start;
};

struct TemporaryMemory
{
    Arena *arena;
    size_t used;
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
    SDL_Renderer *renderer;
    u32 colors[Color_Count];
    Camera camera;
    Map *currentMap;
    SoundList sounds;
    GameMode mode;
    char *dialogue;
    FontMetadata *fontMetadata;

    TextureHandle linkTexture;
    TextureHandle harvestableTreeTexture;
    TextureHandle glowTreeTexture;
    TextureHandle harlodTexture;
    TextureHandle knightTexture;
    TextureHandle flameTexture;
    TextureHandle firePitTexture;

    Sound mudSound;
    // TODO(cjh): Replace SoundList
    // u32 soundCount;
    // Sound *sounds[16];

    Arena worldArena;
    Arena transientArena;
};

#define PushStruct(arena, type) ((type*)pushSize((arena), sizeof(type)))
#define PushSize(arena, size) (pushSize(arena, size))
u8* pushSize(Arena *arena, size_t size);
void startDialogueMode(Game *g, char *dialogue);
void endDialogueMode(Game *g);
void startInventoryMode(Game *g);
void endInventoryMode(Game *g);
#endif
