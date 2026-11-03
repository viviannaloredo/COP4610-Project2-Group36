/*
 * proc.c - /proc/elevator, a consistent snapshot taken under the mutex.
 */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "elevator.h"

#define PROC_NAME "elevator"
#define PROC_SIZE 10000

static struct proc_dir_entry *elevator_proc;

static const char * const state_names[] = {
    "OFFLINE", "IDLE", "LOADING", "UP", "DOWN"
};

/* Append " C3"-style entries for every pet in a list. Caller holds lock. */
static int print_pets(char *buf, int len, struct list_head *head)
{
    struct pet *pet;

    list_for_each_entry(pet, head, list) {
        if (len >= PROC_SIZE - 64)
            break;
        len += scnprintf(buf + len, PROC_SIZE - len, " %c%d",
                         pet_letter(pet->type), pet->destination);
    }
    return len;
}

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
    len = print_pets(buf, len, &onboard);
    len += scnprintf(buf + len, PROC_SIZE - len, "\n\n");

    for (floor = FLOORS; floor >= 1; floor--) {
        int n = 0;

        if (len >= PROC_SIZE - 64)
            break;
        list_for_each_entry(pet, &waiting[floor - 1], list)
            n++;
        len += scnprintf(buf + len, PROC_SIZE - len, "[%c] Floor %d: %d",
                         current_floor == floor ? '*' : ' ', floor, n);
        len = print_pets(buf, len, &waiting[floor - 1]);
        len += scnprintf(buf + len, PROC_SIZE - len, "\n");
    }

    len += scnprintf(buf + len, PROC_SIZE - len,
                     "\nNumber of pets: %d\nNumber of pets waiting: %d\n"
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

int elevator_proc_init(void)
{
    elevator_proc = proc_create(PROC_NAME, 0444, NULL, &elevator_ops);
    return elevator_proc ? 0 : -ENOMEM;
}

void elevator_proc_exit(void)
{
    proc_remove(elevator_proc);
}
