#ifndef GAMEDEV_H
#define GAMEDEV_H

#include "gamedev_platform.h"
#include "gamedev_memory.h"

enum Direction
{
    Direction_Up,
    Direction_Up_Right,
    Direction_Right,
    Direction_Down_Right,
    Direction_Down,
    Direction_Down_Left,
    Direction_Left,
    Direction_Up_Left,

    Direction_COUNT
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
    u64 elapsed;
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

struct Map;
struct Entity;
struct Sprite;

#define Icon_Count 245

struct Game
{
    u32 totalFramesElapsed;
    bool running;
    Camera camera;
    Map *currentMap;
    SoundList sounds;
    GameMode mode;
    char *dialogue;
    FontMetadata fontMetadata;

    Entity *hero;

    void *renderer;
    TextureHandle linkTexture;
    TextureHandle harvestableTreeTexture;
    TextureHandle glowTreeTexture;
    TextureHandle harlodTexture;
    TextureHandle knightTexture;
    TextureHandle flameTexture;
    TextureHandle firePitTexture;
    TextureHandle iconsTexture;

    Sound mudSound;
    // TODO(cjh): Replace SoundList
    // u32 soundCount;
    // Sound *sounds[16];

    Arena worldArena;
    Arena transientArena;
    Sprite icons[Icon_Count];
    Vec4u8 colors[Color_Count];
};

internal void startDialogueMode(Game *g, char *dialogue);
internal void endDialogueMode(Game *g);
internal void startInventoryMode(Game *g);
internal void endInventoryMode(Game *g);
internal void initAnimation(Animation* a, int frames, int ms_delay);
internal void updateAnimation(Animation* a, u64 elapsed_last_frame, b32 active);

global PlatformAPI platform = {};
global RendererAPI rendererAPI = {};
global FontAPI fontAPI = {};
global AudioAPI audioAPI = {};
#endif
