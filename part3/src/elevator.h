/*
 * elevator.h - shared definitions for the pet elevator kernel module.
 * All variables declared here are protected by elevator_lock.
 */
#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#define FLOORS 5
#define MAX_PETS 5
#define MAX_WEIGHT 50
#define NUM_PET_TYPES 4
#define MOVE_SECONDS 2
#define LOAD_SECONDS 1

struct pet {
    struct list_head list;
    int destination;
    int type;
    int weight;
    unsigned long long order;   /* arrival order, used to find oldest request */
};

enum elevator_state {
    OFFLINE,
    IDLE,
    LOADING,
    UP,
    DOWN
};

/* Shared elevator data (defined in elevator_main.c) */
extern struct list_head waiting[FLOORS];   /* FIFO line of pets per floor */
extern struct list_head onboard;           /* pets riding the elevator */
extern struct mutex elevator_lock;
extern wait_queue_head_t elevator_wait;    /* wakes the elevator thread */

extern enum elevator_state state;
extern bool active;
extern bool stopping;
extern int current_floor;
extern int onboard_count;
extern int onboard_weight;
extern int waiting_count;
extern int serviced_count;

/* pets.c */
struct pet *pet_create(int destination, int type, unsigned long long order);
char pet_letter(int type);
void pet_free_list(struct list_head *head);

/* scheduler.c */
int elevator_worker(void *unused);

/* proc.c */
int elevator_proc_init(void);
void elevator_proc_exit(void);

#endif /* ELEVATOR_H */
