#ifndef GD_SOUND_H
#define GD_SOUND_H

struct Sound
{
    u8 is_playing;
    u32 delay;
    u64 last_play_time;
    Mix_Chunk* chunk;
};
#endif
