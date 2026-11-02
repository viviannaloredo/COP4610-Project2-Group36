#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>
#include <linux/time64.h>
#include <linux/mutex.h>

#define ENTRY_NAME "timer"
#define BUF_LEN 128

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("COP4610 timer module");

static struct proc_dir_entry *timer_entry;
static struct timespec64 last_time;
static bool has_last_time;
static DEFINE_MUTEX(timer_lock);

/* Read the current time and time since the last read */
static ssize_t timer_read(struct file *file, char __user *ubuf,
                          size_t count, loff_t *ppos)
{
    char buf[BUF_LEN];
    struct timespec64 now;
    s64 seconds;
    long nanoseconds;
    int len;

    if (*ppos > 0)
        return 0;

    mutex_lock(&timer_lock);
    ktime_get_real_ts64(&now);

    len = scnprintf(buf, sizeof(buf),
                    "current time: %lld.%09ld\n",
                    (long long)now.tv_sec, now.tv_nsec);

    if (has_last_time) {
        seconds = now.tv_sec - last_time.tv_sec;
        nanoseconds = now.tv_nsec - last_time.tv_nsec;

        if (nanoseconds < 0) {
            seconds--;
            nanoseconds += NSEC_PER_SEC;
        }

        len += scnprintf(buf + len, sizeof(buf) - len,
                         "elapsed time: %lld.%09ld\n",
                         (long long)seconds, nanoseconds);
    }

    if (count < (size_t)len) {
        mutex_unlock(&timer_lock);
        return -EINVAL;
    }

    if (copy_to_user(ubuf, buf, len)) {
        mutex_unlock(&timer_lock);
        return -EFAULT;
    }

    last_time = now;
    has_last_time = true;
    *ppos += len;

    mutex_unlock(&timer_lock);
    return len;
}

static const struct proc_ops timer_ops = {
    .proc_read = timer_read,
};

/* Create /proc/timer */
static int __init timer_init(void)
{
    has_last_time = false;

    timer_entry = proc_create(ENTRY_NAME, 0444,
                              NULL, &timer_ops);

    if (!timer_entry)
        return -ENOMEM;

    return 0;
}

/* Remove /proc/timer */
static void __exit timer_exit(void)
{
    proc_remove(timer_entry);
}

module_init(timer_init);
module_exit(timer_exit);
