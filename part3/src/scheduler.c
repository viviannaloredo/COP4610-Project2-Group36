/*
 * scheduler.c - elevator kthread: loading/unloading and movement.
 *
 * Each pass: unload/load at the current floor if needed (1s), go OFFLINE
 * once a stop was requested and the car is empty, otherwise move one floor
 * (2s) toward the next destination: the first rider's destination, or the
 * floor of the oldest waiting pet when the car is empty. The mutex is never
 * held while sleeping.
 */
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "elevator.h"

/* Caller holds elevator_lock. */
static bool can_board_here(void)
{
    struct pet *first;

    if (stopping || onboard_count == MAX_PETS ||
        list_empty(&waiting[current_floor - 1]))
        return false;

    first = list_first_entry(&waiting[current_floor - 1],
                             struct pet, list);
    return onboard_weight + first->weight <= MAX_WEIGHT;
}

/* Caller holds elevator_lock. */
static bool must_unload_here(void)
{
    struct pet *pet;

    list_for_each_entry(pet, &onboard, list) {
        if (pet->destination == current_floor)
            return true;
    }
    return false;
}

/* Unload riders for this floor, then board in FIFO order. Caller holds lock. */
static void service_current_floor(void)
{
    struct pet *pet, *next;

    list_for_each_entry_safe(pet, next, &onboard, list) {
        if (pet->destination == current_floor) {
            list_del(&pet->list);
            onboard_count--;
            onboard_weight -= pet->weight;
            serviced_count++;
            kfree(pet);
        }
    }

    while (can_board_here()) {
        pet = list_first_entry(&waiting[current_floor - 1],
                               struct pet, list);
        list_move_tail(&pet->list, &onboard);
        waiting_count--;
        onboard_count++;
        onboard_weight += pet->weight;
    }
}

/* Caller holds elevator_lock. Choose oldest waiting floor if empty. */
static int next_destination(void)
{
    struct pet *pet;
    unsigned long long oldest = ~0ULL;
    int floor, target = current_floor;

    if (!list_empty(&onboard)) {
        pet = list_first_entry(&onboard, struct pet, list);
        return pet->destination;
    }

    if (stopping)
        return target;

    for (floor = 1; floor <= FLOORS; floor++) {
        if (list_empty(&waiting[floor - 1]))
            continue;
        pet = list_first_entry(&waiting[floor - 1],
                               struct pet, list);
        if (pet->order < oldest) {
            oldest = pet->order;
            target = floor;
        }
    }
    return target;
}

int elevator_worker(void *unused)
{
    int target, direction;

    while (!kthread_should_stop()) {
        /* Sleep while OFFLINE */
        wait_event_interruptible(elevator_wait,
                 kthread_should_stop() || READ_ONCE(active));
        if (kthread_should_stop())
            break;

        mutex_lock(&elevator_lock);
        if (!active) {
            mutex_unlock(&elevator_lock);
            continue;
        }

        /* Deactivate only after every rider is delivered */
        if (stopping && onboard_count == 0) {
            active = false;
            stopping = false;
            state = OFFLINE;
            mutex_unlock(&elevator_lock);
            continue;
        }

        if (must_unload_here() || can_board_here()) {
            state = LOADING;
            mutex_unlock(&elevator_lock);

            ssleep(LOAD_SECONDS);

            mutex_lock(&elevator_lock);
            service_current_floor();
            mutex_unlock(&elevator_lock);
            continue;
        }

        /* Nothing to do: IDLE until a request or stop arrives */
        target = next_destination();
        if (target == current_floor) {
            state = IDLE;
            mutex_unlock(&elevator_lock);
            wait_event_interruptible(elevator_wait,
                     kthread_should_stop() || READ_ONCE(stopping) ||
                     READ_ONCE(waiting_count) > 0);
            continue;
        }

        /* Move one floor toward the target */
        direction = (target > current_floor) ? 1 : -1;
        state = (direction > 0) ? UP : DOWN;
        mutex_unlock(&elevator_lock);

        ssleep(MOVE_SECONDS);

        mutex_lock(&elevator_lock);
        current_floor += direction;
        mutex_unlock(&elevator_lock);
    }
    return 0;
}
