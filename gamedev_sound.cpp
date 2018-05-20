#include "gamedev_sound.h"

Mix_Chunk* sound_load_wav(const char* fname)
{
    Mix_Chunk * result = Mix_LoadWAV(fname);
    if (result == NULL)
    {
        printf("Mix_LoadWAV error: %s\n", Mix_GetError());
        return NULL;
    }
    return result;
}

void sound_play(Sound* s, u64 now)
{
    if (now > s->last_play_time + s->delay)
    {
        Mix_PlayChannel(-1, s->chunk, 0);
        s->last_play_time = SDL_GetTicks();
    }
}

void sound_play_all(SoundList* sl, u64 now)
{
    for (u32 i = 0; i < sl->count; ++i)
    {
        sound_play(sl->items[i], now);
        sl->items[i] = NULL;
    }
    sl->count = 0;
}

void sound_queue(Sound* s, SoundList* sl)
{
    sl->items[sl->count++] = s;
}

void sound_destroy(Sound* s)
{
    Mix_FreeChunk(s->chunk);
}
