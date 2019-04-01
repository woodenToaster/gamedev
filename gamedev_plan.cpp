#include "gamedev_plan.h"

void updatePlan(Entity* e, u32 last_frame_duration)
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
            e->can_move = true;
        }
        else
        {
            e->can_move = false;
        }
    }
}
