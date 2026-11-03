# Pet Elevator

Project 2 for COP4610, Group 36, written in C.

**Group:** Damian Abrego, Vivianna Loredo, Xavier Rosario

## What's in here

The project has three parts:

1. **System call tracing** - We use `strace` to compare an empty C program against one that
   makes five extra system calls.
2. **Timer module** - A kernel module that creates `/proc/timer` and shows the current time
   plus how long it's been since the last read.
3. **Pet elevator** - Three custom system calls (`start_elevator`, `issue_request`,
   `stop_elevator`) and a kernel module that runs a five-floor elevator for pets.

The elevator runs on a kernel thread, keeps track of pets with linked lists, and uses mutexes
to protect shared data. It holds up to five pets or 50 pounds, boards pets in FIFO order,
takes two seconds to move between floors and one second to load or unload, and reports its
status through `/proc/elevator`.

## Division of labor

**Original plan** (submitted on Canvas):

| Section | Assigned to |
| --- | --- |
| Part 1: System call tracing | Vivianna, Xavier |
| Part 2: Timer module | Vivianna, Xavier |
| Part 3A: Adding system calls | Damian, Xavier |
| Part 3B: Kernel compilation | Damian, Xavier |
| Part 3C: Kernel thread | Damian, Xavier |
| Part 3D: Linked lists | Vivianna, Damian |
| Part 3E: Mutexes | Vivianna, Damian |
| Part 3F: Scheduling | Damian, Vivianna |

**What actually happened:**

| Member | Contributions |
| --- | --- |
| Vivianna Loredo | |Contributions Matched the Division(CMD)|
| Xavier Rosario | |CMD|
| Damian Abrego | |CMD|

## File listing

```text
COP4610-Project2-Group36/
├── Makefile
├── README.md
├── .gitignore
├── part1/
│   ├── Makefile
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   └── part1.trace
├── part2/
│   ├── Makefile
│   └── src/
│       ├── Makefile
│       └── my_timer.c
└── part3/
    ├── Makefile
    ├── syscalls.c
    ├── src/
    │   ├── Makefile
    │   ├── elevator.c
    │   ├── elevator.h
    │   ├── elevator_main.c
    │   ├── pets.c
    │   ├── proc.c
    │   └── scheduler.c
    └── tests/
        ├── elevator-test/        (producer.c, consumer.c, wrappers.h, Makefile)
        └── system-calls-test/    (syscheck.c, test-syscalls.c/.h, Makefile)
```

- `Makefile` (root) - builds all three parts.
- `part1/` - the two C programs and their `strace` output.
- `part2/src/my_timer.c` - the timer module that creates `/proc/timer`.
- `part3/syscalls.c` - the three system call wrappers.
- `part3/src/elevator_main.c` - module setup, teardown, and the elevator kernel thread.
- `part3/src/pets.c` - pet requests and the linked lists for floors and the elevator.
- `part3/src/scheduler.c` - how the elevator moves and picks up pets.
- `part3/src/proc.c` - the `/proc/elevator` output.
- `part3/src/elevator.h` and `elevator.c` - shared definitions and elevator state.
- `part3/tests/` - the course-provided producer/consumer and system call tests.

## Building and running

You'll need a Linux VM (we used Ubuntu Server 24.04 on x86-64) with GCC, Make, `strace`,
kernel headers, and `sudo`. Kernel modules won't run directly on macOS or Windows.

### Part 1

```bash
cd part1
make
make trace
```

### Part 2

```bash
cd part2
make
sudo insmod src/my_timer.ko
cat /proc/timer
sudo rmmod my_timer
```

Read `/proc/timer` a few times with a `sleep` in between to see the elapsed time change.
Removing the module removes the proc entry.

### Part 3

Part 3 needs a custom kernel (we used Linux 6.16.12) because of the added system calls.
Here's what we changed in the kernel source:

1. Copy `part3/syscalls.c` into the kernel's `syscalls/` directory and add a Makefile there
   with `obj-y += syscalls.o`.
2. Add these entries to `arch/x86/entry/syscalls/syscall_64.tbl`:
   - `548 common start_elevator sys_start_elevator`
   - `549 common issue_request sys_issue_request`
   - `550 common stop_elevator sys_stop_elevator`
3. Declare all three functions in `include/linux/syscalls.h`.
4. Add `syscalls/` to `core-y` in the kernel's top-level Makefile.
5. Build and install the kernel, reboot, and check `uname -r` to make sure you're on the
   custom one.

Then, from the repo root:

```bash
cd part3
make
sudo insmod elevator.ko

cd tests/elevator-test
make
./consumer --start
./producer 10
watch -n 1 cat /proc/elevator
./consumer --stop

sudo rmmod elevator
```

### Cleaning up

Run `make clean` in any part's folder, or at the root to clean everything. Kernel module
cleanup needs the matching kernel build directory to be installed.

## Development log

### Vivianna Loredo

| Date | Work completed |
| --- | --- |
| 2026-11-02 | Pushed the initial files: Parts 1 and 2, elevator, syscalls, Makefiles, README. |
| 2026-11-02 | Fixed the elevator loading time (commit "Correct elevator loading timing"). |

### Xavier Rosario

| Date | Work completed |
| --- | --- |
| 2026-11-02 | Pushed the split-up Part 3 source files and the provided tests in `part3/tests/`. |

### Damian Abrego

| Date | Work completed |
| --- | --- |
| 2026-11-02 | Revised the README: project description, development log, and meeting sections. |

## Group meetings

| Date | Attendees | Format | Discussion |
| --- | --- | --- | --- |
| 2026-11-02 | Vivianna, Xavier, Damian | |zoom,group chat|| Compilation and testing status. |

## Testing

**Part 1:** `empty.trace` has 30 system calls and `part1.trace` has 35, so the second program
makes exactly five extra.

**Part 2:** The module builds and loads on Ubuntu's 6.8 kernel, `/proc/timer` shows the
current time on the first read, and later reads show elapsed times that match our `sleep`
delays (about 2 and 3 seconds). Unloading removes the entry.

**Part 3:** The elevator source compiles, but we're still working on full testing on the
custom kernel.
