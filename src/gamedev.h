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

struct SoundList
{
    u32 count;
    Sound* items[MAX_SOUNDS_PER_FRAME];
};

internal void queueSound(SoundList *sl, Sound *s);
internal void playQueuedSounds(SoundList *sl, u64 now);

enum GameMode
{
    GameMode_Playing,
    GameMode_Dialogue,
    GameMode_Inventory,
    GameMode_Count
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
struct Entity;

struct Game
{
    u32 dt;
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
    FontMetadata fontMetadata;

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
internal void startDialogueMode(Game *g, char *dialogue);
internal void endDialogueMode(Game *g);
internal void startInventoryMode(Game *g);
internal void endInventoryMode(Game *g);
internal void initAnimation(Animation* a, int frames, int ms_delay);
internal void updateAnimation(Animation* a, u32 elapsed_last_frame, b32 active);

global_variable PlatformAPI platform = {};
global_variable RendererAPI rendererAPI = {};
global_variable FontAPI fontAPI = {};
global_variable AudioAPI audioAPI = {};
global_variable b32 globalRunning = 0;
#endif
