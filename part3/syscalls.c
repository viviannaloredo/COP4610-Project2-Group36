#include <linux/errno.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/mutex.h>

/* Function pointers for the elevator module */
int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;

EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

/* Protect every call against module removal. */
DEFINE_MUTEX(elevator_syscall_lock);
EXPORT_SYMBOL(elevator_syscall_lock);

/* Start the elevator */
SYSCALL_DEFINE0(start_elevator)
{
    long result = -ENOSYS;

    mutex_lock(&elevator_syscall_lock);
    if (STUB_start_elevator != NULL)
        result = STUB_start_elevator();
    mutex_unlock(&elevator_syscall_lock);

    return result;
}

/* Add a pet request */
SYSCALL_DEFINE3(issue_request, int, start_floor,
                int, destination_floor, int, type)
{
    long result = -ENOSYS;

    mutex_lock(&elevator_syscall_lock);
    if (STUB_issue_request != NULL)
        result = STUB_issue_request(start_floor,
                                  destination_floor, type);
    mutex_unlock(&elevator_syscall_lock);

    return result;
}

/* Stop the elevator */
SYSCALL_DEFINE0(stop_elevator)
{
    long result = -ENOSYS;

    mutex_lock(&elevator_syscall_lock);
    if (STUB_stop_elevator != NULL)
        result = STUB_stop_elevator();
    mutex_unlock(&elevator_syscall_lock);

    return result;
}
