#include "gamedev_plan.h"
#include "gamedev_entity.h"

void plan_update(Entity* e, u32 last_frame_duration)
{
    e->plan.current_delay += last_frame_duration;

    if (e->plan.current_delay >= e->plan.move_delay)
    {
        e->plan.current_delay = 0;
        e->plan.waiting = !e->plan.waiting;
        e->plan.mv_dir = (CardinalDir)(rand() % 4);
        e->plan.move_delay = (rand() % 2000) + 1000;

        if (!e->plan.waiting)
        {
            e->can_move = GD_TRUE;
        }
        else
        {
            e->can_move = GD_FALSE;
        }
    }
}
