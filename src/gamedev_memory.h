#ifndef GAMEDEV_MEMORY_H
#define GAMEDEV_MEMORY_H

#include "gamedev_platform.h"

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

#define PUSH_STRUCT(arena, type) ((type *)pushSize((arena), sizeof(type)))
#define PUSH_CLEARED_STRUCT(arena, type) ((type *)pushSizeAndClear((arena), sizeof(type)))
#define PUSH_ARRAY(arena, type, count) ((type *)pushSize((arena), sizeof(type) * (count)))

void initArena(Arena *arena, size_t bytes, u8 *start)
{
    arena->start = start;
    arena->maxCap = bytes;
    arena->used = 0;
    arena->tmpCount = 0;
}

u8* pushSize(Arena *arena, size_t size)
{
    assert(size + arena->used < arena->maxCap);
    u8* result = arena->start + arena->used;
    arena->used += size;
    return result;
}

u8* pushSizeAndClear(Arena *arena, size_t size)
{
    assert(size + arena->used < arena->maxCap);
    u8* result = arena->start + arena->used;
    arena->used += size;
    memset(result, 0, size);
    return result;
}

internal TemporaryMemory beginTemporaryMemory(Arena *arena);
internal void endTemporaryMemory(TemporaryMemory tempMem);

#endif
