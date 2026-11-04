/*
 * elevator_main.c - module init/exit and the handlers for system calls
 * 548 (start_elevator), 549 (issue_request), and 550 (stop_elevator).
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("COP4610 pet elevator module");

/* These pointers are exported by syscalls/syscalls.c in our custom kernel. */
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);
extern struct mutex elevator_syscall_lock;

/* Shared elevator data, protected by elevator_lock */
struct list_head waiting[FLOORS];
LIST_HEAD(onboard);
DEFINE_MUTEX(elevator_lock);
DECLARE_WAIT_QUEUE_HEAD(elevator_wait);

enum elevator_state state = OFFLINE;
bool active;
bool stopping;
int current_floor = 1;
int onboard_count;
int onboard_weight;
int waiting_count;
int serviced_count;

static unsigned long long next_order;
static struct task_struct *elevator_task;

/* 548: returns 1 if already active, 0 on success. */
static int elevator_start(void)
{
    mutex_lock(&elevator_lock);
    if (active) {
        mutex_unlock(&elevator_lock);
        return 1;
    }

    active = true;
    stopping = false;
    state = IDLE;
    current_floor = 1;
    mutex_unlock(&elevator_lock);
    wake_up(&elevator_wait);
    return 0;
}

/* 549: returns 1 if invalid, 0 on success. Requests may queue while offline. */
static int elevator_request(int start, int destination, int type)
{
    struct pet *new_pet;

    if (start < 1 || start > FLOORS ||
        destination < 1 || destination > FLOORS ||
        start == destination || type < 0 || type >= NUM_PET_TYPES)
        return 1;

    /* kmalloc may sleep, so allocate before taking the lock */
    new_pet = pet_create(destination, type, 0);
    if (!new_pet)
        return -ENOMEM;

    mutex_lock(&elevator_lock);
    new_pet->order = next_order++;
    list_add_tail(&new_pet->list, &waiting[start - 1]);
    waiting_count++;
    mutex_unlock(&elevator_lock);
    wake_up(&elevator_wait);
    return 0;
}

/* 550: returns 1 if already deactivating, 0 otherwise. */
static int elevator_stop(void)
{
    mutex_lock(&elevator_lock);
    if (stopping) {
        mutex_unlock(&elevator_lock);
        return 1;
    }

    if (active)
        stopping = true;   /* thread delivers riders, then goes OFFLINE */
    mutex_unlock(&elevator_lock);
    wake_up(&elevator_wait);
    return 0;
}

static int __init elevator_init(void)
{
    int floor, err;

    for (floor = 0; floor < FLOORS; floor++)
        INIT_LIST_HEAD(&waiting[floor]);

    err = elevator_proc_init();
    if (err)
        return err;

    elevator_task = kthread_run(elevator_worker, NULL, "pet_elevator");
    if (IS_ERR(elevator_task)) {
        err = PTR_ERR(elevator_task);
        elevator_task = NULL;
        elevator_proc_exit();
        return err;
    }

    mutex_lock(&elevator_syscall_lock);
    STUB_start_elevator = elevator_start;
    STUB_issue_request = elevator_request;
    STUB_stop_elevator = elevator_stop;
    mutex_unlock(&elevator_syscall_lock);
    return 0;
}

static void __exit elevator_exit(void)
{
    int floor;

    /* Stop accepting system calls before tearing anything down */
    mutex_lock(&elevator_syscall_lock);
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;
    mutex_unlock(&elevator_syscall_lock);

    elevator_proc_exit();
    kthread_stop(elevator_task);

    mutex_lock(&elevator_lock);
    pet_free_list(&onboard);
    for (floor = 0; floor < FLOORS; floor++)
        pet_free_list(&waiting[floor]);
    mutex_unlock(&elevator_lock);
}

module_init(elevator_init);
module_exit(elevator_exit);
