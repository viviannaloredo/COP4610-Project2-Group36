#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define FLOORS 5
#define MAX_PETS 5
#define MAX_WEIGHT 50
#define PROC_SIZE 10000

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("COP4610 pet elevator module");

/* These pointers are exported by syscalls/syscalls.c in our custom kernel. */
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);
extern struct mutex elevator_syscall_lock;

struct pet {
    struct list_head list;
    int destination;
    int type;
    int weight;
    unsigned long long order;
};

enum elevator_state {
    OFFLINE,
    IDLE,
    LOADING,
    UP,
    DOWN
};

static struct list_head waiting[FLOORS];
static LIST_HEAD(onboard);
static DEFINE_MUTEX(elevator_lock);
static DECLARE_WAIT_QUEUE_HEAD(elevator_wait);
static struct task_struct *elevator_task;
static struct proc_dir_entry *elevator_proc;

static enum elevator_state state = OFFLINE;
static bool active;
static bool stopping;
static int current_floor = 1;
static int onboard_count;
static int onboard_weight;
static int waiting_count;
static int serviced_count;
static unsigned long long next_order;

static const int pet_weights[] = { 3, 14, 10, 16 };
static const char pet_letters[] = { 'C', 'P', 'H', 'D' };
static const char * const state_names[] = {
    "OFFLINE", "IDLE", "LOADING", "UP", "DOWN"
};

/* Called by system call 548. */
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

/* Called by system call 549. Requests may queue while offline. */
static int elevator_request(int start, int destination, int type)
{
    struct pet *new_pet;

    if (start < 1 || start > FLOORS ||
        destination < 1 || destination > FLOORS ||
        start == destination || type < 0 || type > 3)
        return 1;

    new_pet = kmalloc(sizeof(*new_pet), GFP_KERNEL);
    if (!new_pet)
        return -ENOMEM;

    new_pet->destination = destination;
    new_pet->type = type;
    new_pet->weight = pet_weights[type];

    mutex_lock(&elevator_lock);
    new_pet->order = next_order++;
    list_add_tail(&new_pet->list, &waiting[start - 1]);
    waiting_count++;
    mutex_unlock(&elevator_lock);
    wake_up(&elevator_wait);
    return 0;
}

/* Called by system call 550. Finish delivering onboard pets, then stop. */
static int elevator_stop(void)
{
    mutex_lock(&elevator_lock);
    if (!active || stopping) {
        mutex_unlock(&elevator_lock);
        return 1;
    }

    stopping = true;
    mutex_unlock(&elevator_lock);
    wake_up(&elevator_wait);
    return 0;
}

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

/* Caller holds elevator_lock. */
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

static int elevator_worker(void *unused)
{
    int target, direction;
    bool loading;

    while (!kthread_should_stop()) {
        wait_event_interruptible(elevator_wait,
                 kthread_should_stop() || READ_ONCE(active));
        if (kthread_should_stop())
            break;

        mutex_lock(&elevator_lock);
        if (!active) {
            mutex_unlock(&elevator_lock);
            continue;
        }

        if (stopping && onboard_count == 0) {
            active = false;
            stopping = false;
            state = OFFLINE;
            mutex_unlock(&elevator_lock);
            continue;
        }

        loading = must_unload_here() || can_board_here();
        if (loading) {
            state = LOADING;
            mutex_unlock(&elevator_lock);

            ssleep(10);

            mutex_lock(&elevator_lock);
            service_current_floor();
            mutex_unlock(&elevator_lock);
            continue;
        }

        target = next_destination();
        if (target == current_floor) {
            state = IDLE;
            mutex_unlock(&elevator_lock);
            wait_event_interruptible(elevator_wait,
                 kthread_should_stop() || READ_ONCE(stopping) ||
                 READ_ONCE(waiting_count) > 0);
            continue;
        }

        direction = (target > current_floor) ? 1 : -1;
        state = (direction > 0) ? UP : DOWN;
        mutex_unlock(&elevator_lock);

        ssleep(2);

        mutex_lock(&elevator_lock);
        current_floor += direction;
        mutex_unlock(&elevator_lock);
    }
    return 0;
}

/* The /proc file presents a consistent snapshot under the mutex. */
static ssize_t elevator_read(struct file *file, char __user *user_buf,
                             size_t count, loff_t *position)
{
    char *buf;
    int len = 0, floor;
    ssize_t result;
    struct pet *pet;

    if (*position > 0)
        return 0;

    buf = kmalloc(PROC_SIZE, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    mutex_lock(&elevator_lock);
    len += scnprintf(buf + len, PROC_SIZE - len,
                     "Elevator state: %s\nCurrent floor: %d\n"
                     "Current load: %d lbs\nElevator status:",
                     state_names[state], current_floor, onboard_weight);
    if (list_empty(&onboard))
        len += scnprintf(buf + len, PROC_SIZE - len, " empty");
    else
        list_for_each_entry(pet, &onboard, list) {
            if (len >= PROC_SIZE - 64)
                break;
            len += scnprintf(buf + len, PROC_SIZE - len,
                             " %c%d", pet_letters[pet->type],
                             pet->destination);
        }
    len += scnprintf(buf + len, PROC_SIZE - len, "\n");

    for (floor = FLOORS; floor >= 1; floor--) {
        int n = 0;

        if (len >= PROC_SIZE - 64)
            break;
        list_for_each_entry(pet, &waiting[floor - 1], list)
            n++;
        len += scnprintf(buf + len, PROC_SIZE - len,
                         "[%c] Floor %d: %d", current_floor == floor ? '*' : ' ',
                         floor, n);
        list_for_each_entry(pet, &waiting[floor - 1], list) {
            if (len >= PROC_SIZE - 64)
                break;
            len += scnprintf(buf + len, PROC_SIZE - len,
                             " %c%d", pet_letters[pet->type],
                             pet->destination);
        }
        len += scnprintf(buf + len, PROC_SIZE - len, "\n");
    }
    len += scnprintf(buf + len, PROC_SIZE - len,
                     "Number of pets: %d\nNumber of pets waiting: %d\n"
                     "Number of pets serviced: %d\n",
                     onboard_count, waiting_count, serviced_count);
    mutex_unlock(&elevator_lock);

    result = simple_read_from_buffer(user_buf, count, position, buf, len);
    kfree(buf);
    return result;
}

static const struct proc_ops elevator_ops = {
    .proc_read = elevator_read,
};

static int __init elevator_init(void)
{
    int floor;

    for (floor = 0; floor < FLOORS; floor++)
        INIT_LIST_HEAD(&waiting[floor]);

    elevator_proc = proc_create("elevator", 0444, NULL, &elevator_ops);
    if (!elevator_proc)
        return -ENOMEM;

    elevator_task = kthread_run(elevator_worker, NULL, "pet_elevator");
    if (IS_ERR(elevator_task)) {
        int err = PTR_ERR(elevator_task);
        elevator_task = NULL;
        proc_remove(elevator_proc);
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
    struct pet *pet, *next;
    int floor;

    /* Wait for active syscalls to return before removing callbacks. */
    mutex_lock(&elevator_syscall_lock);
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;
    mutex_unlock(&elevator_syscall_lock);
    proc_remove(elevator_proc);
    kthread_stop(elevator_task);

    mutex_lock(&elevator_lock);
    list_for_each_entry_safe(pet, next, &onboard, list) {
        list_del(&pet->list);
        kfree(pet);
    }
    for (floor = 0; floor < FLOORS; floor++) {
        list_for_each_entry_safe(pet, next, &waiting[floor], list) {
            list_del(&pet->list);
            kfree(pet);
        }
    }
    mutex_unlock(&elevator_lock);
}

module_init(elevator_init);
module_exit(elevator_exit);
