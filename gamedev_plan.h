#ifndef GD_PLAN_H
#define GD_PLAN_H

struct Plan
{
    CardinalDir mv_dir;
    u8 do_plan;
    u8 waiting;
    u32 move_delay;
    u32 current_delay;
};
#endif
