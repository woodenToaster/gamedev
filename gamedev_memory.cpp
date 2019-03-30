#include "gamedev_memory.h"

void initArena(Arena* arena, size_t bytes)
{
    arena->start = (char*)malloc(bytes);
    arena->next = arena->start;
    arena->max_cap = bytes;
}

void destroyArena(Arena* arena)
{
    free(arena->start);
    arena->start = NULL;
    arena->next = NULL;
    arena->used = 0;
    arena->max_cap = 0;
}

char* pushSize(Arena* arena, size_t size)
{
    assert(size + arena->used < arena->max_cap);
    char* ptr = arena->next;
    arena->next += size;
    arena->used += size;
    memset(ptr, 0, size);
    return ptr;
}
