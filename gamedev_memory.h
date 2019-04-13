#ifndef GAMEDEV_MEMORY_H
#define GAMEDEV_MEMORY_H

struct Arena
{
    u8* start;
    u8* next;
    size_t used;
    size_t maxCap;
};

#define PushStruct(arena, type) ((type*)pushSize((arena), sizeof(type)))

#endif
