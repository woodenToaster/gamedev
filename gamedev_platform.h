#ifndef GAMEDEV_PLATFORM_H
#define GAMEDEV_PLATFORM_H

#include <stdint.h>

#define MAX_CONTROLLERS 2

#define internal static
#define local_persist static
#define global_variable static

#define InvalidCodePath assert(!"InvalidCodePath")

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t b32;

#define MEGABYTES(n) ((n) * 1024 * 1024)
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

struct EntireFile
{
    u8 *contents;
    u64 size;
};

struct TextureHandle
{
    void *texture;
};

struct Game;
typedef  EntireFile PlatformReadEntireFile(char *);
typedef void PlatformFreeFileMemory(EntireFile *);
typedef u32 PlatformGetTicks();

// TODO(cjh): Should this be part of platformAPI? Probably not since it takes a Game argument
typedef TextureHandle platformCreateTextureFromGreyscaleBitmap(Game *g, u8 *bitmap, i32 width, i32 height);

struct PlatformAPI
{
    PlatformReadEntireFile *readEntireFile;
    PlatformFreeFileMemory *freeFileMemory;
    PlatformGetTicks *getTicks;
};

enum Key
{
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ESCAPE,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_SPACE,
    KEY_COUNT
};

enum Buttons
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_COUNT
};

struct Input
{
    b32 keyPressed[KEY_COUNT];
    b32 keyDown[KEY_COUNT];
    b32 buttonPressed[BUTTON_COUNT];
    b32 buttonDown[BUTTON_COUNT];
    f32 stickX;
    f32 stickY;
};
#endif
