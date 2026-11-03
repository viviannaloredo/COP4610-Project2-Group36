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
| Vivianna Loredo | |
| Xavier Rosario | |
| Damian Abrego | |

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

Development Log
EXAMPLE ENTRIES — REPLACE BEFORE SUBMISSION. The dates and individual work notes below are sample wording based on the original division of labor. They are not verified records of work performed by the named members. Replace them with each person's real dates and contributions. The October 13 division-of-labor submission and November 2 local development/tests are the dates that have been established.

Vivianna Loredo
Date	Work Completed / Notes (Example)
2026-10-13	Reviewed the project instructions and the sections assigned to me, including Part 1, the timer module, linked lists, mutexes, and the scheduling algorithm. The division-of-labor document was submitted on Canvas.
2026-10-18	Worked on the system-call tracing programs and compared the trace files to check that Part 1 made exactly five additional calls.
2026-10-23	Reviewed the timer module, especially how /proc/timer prints the current time and calculates the elapsed time after another read.
2026-10-28	Worked on the linked-list and mutex portions of the elevator and checked how pet requests should be stored and protected.
2026-11-02	Helped review the elevator's scheduling rules, checked the Part 1 and Part 2 test results, and worked on the final README and repository organization.
Damian Abrego
Date	Work Completed / Notes (Example)
2026-10-13	Reviewed the division of labor and the assigned kernel-related sections, including system calls, kernel compilation, threads, linked lists, mutexes, and elevator scheduling.
2026-10-18	Worked through the required system-call registration changes and how the wrappers connect to the elevator module.
2026-10-23	Reviewed the modified kernel configuration and compilation process and checked the files needed to register the elevator calls.
2026-10-28	Worked on the kernel thread and linked-list behavior, including moving between floors and keeping track of waiting and onboard pets.
2026-11-02	Reviewed mutex synchronization and scheduling behavior and checked the remaining kernel integration work before submission.
Xavier Rosario
Date	Work Completed / Notes (Example)
2026-10-13	Reviewed the project requirements and the assigned sections, including Part 1, Part 2, adding system calls, kernel compilation, and the elevator thread.
2026-10-18	Worked on the two Part 1 programs and used strace to compare the baseline program with the five additional calls.
2026-10-23	Worked on the timer module and checked the current-time output, elapsed-time calculation, and cleanup when unloading.
2026-10-28	Reviewed the custom kernel configuration and system-call wrappers and checked how they would be used by the elevator module.
2026-11-02	Helped review the elevator thread and compilation status and checked the remaining testing steps needed for the demonstration.
Meetings
EXAMPLE MEETING LOG — REPLACE BEFORE SUBMISSION. These are examples of how to document group meetings, not a record that the meetings occurred. Update the dates, attendees, format, and discussion to match actual meetings.

Example Date	Example Attendees	Format	Discussion / Outcome (Example)
2026-10-13	Vivianna Loredo, Xavier Rosario, Damian Abrego	[Confirm format]	Reviewed the project requirements and went through the division of labor. Discussed which tasks depended on the modified kernel and which ones could be tested separately.
2026-10-22	Vivianna Loredo, Xavier Rosario, Damian Abrego	[Confirm format]	Discussed progress on Part 1 and the timer module and checked the steps required to add the three new system calls.
2026-10-29	Vivianna Loredo, Xavier Rosario, Damian Abrego	[Confirm format]	Reviewed the elevator thread, linked lists, mutexes, scheduling requirements, and remaining kernel build tasks.
2026-11-02	Vivianna Loredo, Xavier Rosario, Damian Abrego	[Confirm format]	Discussed the current compilation and testing status and what still needed to be finished before the demonstration.

## Testing

**Part 1:** `empty.trace` has 30 system calls and `part1.trace` has 35, so the second program
makes exactly five extra.

**Part 2:** The module builds and loads on Ubuntu's 6.8 kernel, `/proc/timer` shows the
current time on the first read, and later reads show elapsed times that match our `sleep`
delays (about 2 and 3 seconds). Unloading removes the entry.

**Part 3:** The elevator source compiles, but we're still working on full testing on the
custom kernel.
