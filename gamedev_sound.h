#ifndef GD_SOUND_H
#define GD_SOUND_H

#define MAX_SOUNDS_PER_FRAME 20

struct Sound
{
    u8 is_playing;
    u32 delay;
    u64 last_play_time;
    Mix_Chunk* chunk;
};

struct SoundList
{
    u32 count;
    Sound* items[MAX_SOUNDS_PER_FRAME];
};
#endif
