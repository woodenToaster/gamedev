#ifndef GAMEDEV_H
#define GAMEDEV_H

#include "gamedev_platform.h"

enum Direction
{
    DIR_UP,
    DIR_UP_RIGHT,
    DIR_RIGHT,
    DIR_DOWN_RIGHT,
    DIR_DOWN,
    DIR_DOWN_LEFT,
    DIR_LEFT,
    DIR_UP_LEFT,
    DIR_COUNT
};

struct Camera
{
    i32 maxX;
    i32 maxY;
    i32 yPixelMovementThreshold;
    i32 xPixelMovementThreshold;
    Rect viewport;
    Point startingPos;
};

struct Animation
{
    int totalFrames;
    int currentFrame;
    u32 delay;
    u32 elapsed;
};

#define MAX_SOUNDS_PER_FRAME 20

struct Sound
{
    u8 is_playing;
    u32 delay;
    u64 last_play_time;
    Mix_Chunk* chunk;
};

struct SoundList
{
    u32 count;
    Sound* items[MAX_SOUNDS_PER_FRAME];
};

struct CodepointMetadata
{
    i32 advance;
    i32 leftSideBearing;
    i32 x0;
    i32 y0;
    i32 x1;
    i32 y1;
    f32 xShift;
};

struct FontMetadata
{
    i32 baseline;
    f32 size;
    f32 scale;
    i32 ascent;
    i32 descent;
    i32 lineGap;
    stbtt_fontinfo info;
    TextureHandle textures[128];
    CodepointMetadata codepointMetadata[128];

};

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
    u64 transientStorageSize;
    u32 currentTickCount;
    u32 dt;
    u32 targetMsPerFrame;
    b32 isInitialized;
    void *permanentStorage;
    void *transientStorage;
    PlatformAPI platformAPI;
    RendererAPI rendererAPI;

    // TODO(cjh): Get assets out of here
    TextureHandle linkTexture;
    TextureHandle harvestableTreeTexture;
    TextureHandle flameTexture;
    TextureHandle firePitTexture;
    TextureHandle glowTreeTexture;
    Sound mudSound;
    FontMetadata fontMetadata;

    u32 colors[Color_Count];
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

struct Map;
struct FontMetadata;
struct Entity;

struct Game
{
    u32 dt;
    i32 screenWidth;
    i32 screenHeight;
    i32 targetFps;
    u32 totalFramesElapsed;
    bool running;
    RendererHandle renderer;
    u32 *colors;
    Camera camera;
    Map *currentMap;
    SoundList sounds;
    GameMode mode;
    char *dialogue;
    FontMetadata *fontMetadata;

    Entity *hero;

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
void initAnimation(Animation* a, int frames, int ms_delay);

global_variable PlatformAPI platform = {};
global_variable RendererAPI rendererAPI = {};
global_variable b32 globalRunning = 0;
#endif
