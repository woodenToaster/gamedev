#include "gamedev_sound.h"

Mix_Chunk *loadWav(const char *fname)
{
    Mix_Chunk *result = Mix_LoadWAV(fname);
    if (result == NULL)
    {
        printf("Mix_LoadWAV error: %s\n", Mix_GetError());
        return NULL;
    }
    return result;
}

void playSound(Sound *s, u64 now)
{
    if (now > s->last_play_time + s->delay)
    {
        Mix_PlayChannel(-1, s->chunk, 0);
        s->last_play_time = platform.getTicks();
    }
}

void playQueuedSounds(SoundList *sl, u64 now)
{
    for (u32 i = 0; i < sl->count; ++i)
    {
        playSound(sl->items[i], now);
        sl->items[i] = NULL;
    }
    sl->count = 0;
}

void queueSound(SoundList *sl, Sound *s)
{
    sl->items[sl->count++] = s;
}

void destroySound(Sound *s)
{
    Mix_FreeChunk(s->chunk);
}
