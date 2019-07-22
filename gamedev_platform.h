#ifndef GAMEDEV_PLATFORM_H
#define GAMEDEV_PLATFORM_H

#include <stdint.h>

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
#endif
