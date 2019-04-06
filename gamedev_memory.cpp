#include "gamedev_memory.h"

void initArena(Arena *arena, size_t bytes)
{
    arena->start = (u8*)malloc(bytes);
    arena->next = arena->start;
    arena->maxCap = bytes;
}

void destroyArena(Arena *arena)
{
    free(arena->start);
    arena->start = NULL;
    arena->next = NULL;
    arena->used = 0;
    arena->maxCap = 0;
}

u8* pushSize(Arena *arena, size_t size)
{
    assert(size + arena->used < arena->maxCap);
    u8* ptr = arena->next;
    arena->next += size;
    arena->used += size;
    memset(ptr, 0, size);
    return ptr;
}
