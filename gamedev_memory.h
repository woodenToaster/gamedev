#ifndef GAMEDEV_MEMORY_H
#define GAMEDEV_MEMORY_H

struct Arena
{
    char* start;
    char* next;
    size_t used;
    size_t max_cap;
};

#endif
