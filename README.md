# Pet Elevator

## Description

This project implements a pet elevator in C for COP4610 Operating Systems. The project is divided into three parts. Part 1 uses `strace` to compare the system calls made by an empty program and a program containing five additional system calls. Part 2 creates a Linux kernel module that displays the current time and the time elapsed since the previous read. Part 3 adds three custom system calls to the Linux kernel and uses them to control a pet elevator module.

The elevator uses a kernel thread to move between floors, linked lists to keep track of pets, and mutexes to protect shared information. The parts were organized using the group's original division of labor, with several sections assigned to two members so they could work on related functionality together.

## Group Information

- Group Number: 36
- Course: COP4610 - Operating Systems
- Project: Project 2

## Group Members

- Damian Abrego
- Vivianna Loredo
- Xavier Rosario

---

# Division of Labor

The following is the original division of labor submitted on **October 13, 2026**, as shown in the Canvas submission. These are assigned responsibilities, not a claim that each part has been completed by the listed members.

## Part 1: System Call Tracing

Responsibilities: Create the empty C program and the second C program containing five additional system calls, generate the `strace` files, and compare the results.

Assigned to: Vivianna Loredo and Xavier Rosario

## Part 2: Timer Kernel Module

Responsibilities: Create the `my_timer` kernel module, add `/proc/timer`, display the current and elapsed time, and make sure the proc entry is removed when the module is unloaded.

Assigned to: Vivianna Loredo and Xavier Rosario

## Part 3A: Adding System Calls

Responsibilities: Add the three elevator system calls to the kernel source, register their syscall numbers, and connect the system calls to the elevator module.

Assigned to: Damian Abrego and Xavier Rosario

## Part 3B: Kernel Compilation

Responsibilities: Configure the modified Linux kernel, compile and install it, and verify that it boots with the new system calls.

Assigned to: Damian Abrego and Xavier Rosario

## Part 3C: Threads

Responsibilities: Implement the kernel thread that manages elevator movement, loading, unloading, and stopping.

Assigned to: Damian Abrego and Xavier Rosario

## Part 3D: Linked Lists

Responsibilities: Store the waiting pets on each floor and the pets inside the elevator using linked lists. Handle adding, removing, and freeing pet requests.

Assigned to: Vivianna Loredo and Damian Abrego

## Part 3E: Mutexes

Responsibilities: Protect shared elevator data and coordinate access to the elevator state, pet lists, and system-call handlers.

Assigned to: Vivianna Loredo and Damian Abrego

## Part 3F: Scheduling Algorithm

Responsibilities: Decide how the elevator moves between floors and services requests while following the required FIFO, weight, and capacity rules.

Assigned to: Damian Abrego and Vivianna Loredo

## Division of Labor After Completion

**To complete before final submission:** Confirm with each group member which of the assigned sections they actually implemented, reviewed, or tested. Record any changes from the original assignments here.

- Vivianna Loredo: [Add verified contributions]
- Xavier Rosario: [Add verified contributions]
- Damian Abrego: [Add verified contributions]

---

# Features

The project is designed to support:

- System-call tracing with two C programs and saved `strace` output
- A timer kernel module with a `/proc/timer` entry
- Displaying current time and elapsed time between reads
- Removing `/proc/timer` when the timer module is unloaded
- Three custom elevator system calls: `start_elevator`, `issue_request`, and `stop_elevator`
- A five-floor pet elevator
- A limit of five pets and 50 pounds inside the elevator
- FIFO boarding of waiting pets
- A kernel thread for elevator movement
- Linked lists for pets waiting on each floor and riding the elevator
- Mutexes to protect shared data
- A `/proc/elevator` entry for elevator status
- Two seconds to move between floors and ten seconds for loading/unloading

Part 3's complete behavior remains subject to integration testing on the modified kernel.

---

# File Listing

```text
COP4610-Project2-Group36/
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   │   ├── my_timer.c
│   │   └── Makefile
│   └── Makefile
├── part3/
│   ├── src/
│   │   ├── elevator.c
│   │   └── Makefile
│   ├── syscalls.c
│   └── Makefile
├── .gitignore
├── Makefile
└── README.md
```

## File Descriptions

- `part1/empty.c` - Contains the empty C program used as the baseline for system-call tracing.
- `part1/part1.c` - Makes five additional system calls for comparison with the empty program.
- `part1/empty.trace` and `part1/part1.trace` - Store the results from running `strace` on both programs.
- `part1/Makefile` - Compiles the Part 1 programs and generates their traces.
- `part2/src/my_timer.c` - Creates `/proc/timer` and keeps track of the current and elapsed time.
- `part2/src/Makefile` and `part2/Makefile` - Build the timer kernel module.
- `part3/src/elevator.c` - Implements the elevator state, pets, scheduling logic, kernel thread, and proc output.
- `part3/syscalls.c` - Defines the elevator system-call wrappers and shared callback pointers.
- `part3/src/Makefile` and `part3/Makefile` - Build the elevator kernel module.
- `Makefile` - Contains targets for building the separate project parts.
- `.gitignore` - Excludes generated executables, object files, and compiled kernel modules.

---

# How to Compile and Execute

## Requirements

The project is written in C and developed using Ubuntu Server 24.04 LTS on an x86-64 Linux virtual machine. It uses GCC, Make, `strace`, Linux kernel headers, and kernel-module build tools. Part 2 was tested on Ubuntu's 6.8 kernel. Part 3 requires the modified Linux 6.16.12 kernel containing the custom elevator system calls.

The system must allow `sudo` for installing and removing kernel modules. Simply cloning the GitHub repository on macOS or Windows is not enough to run kernel modules directly; those users need a compatible Linux environment.

## Part 1: System Call Tracing

From the project directory:

```bash
cd part1
make
make trace
```

To compare the trace counts:

```bash
grep -Ec '^[a-zA-Z_][a-zA-Z0-9_]*\(' empty.trace
grep -Ec '^[a-zA-Z_][a-zA-Z0-9_]*\(' part1.trace
```

## Part 2: Timer Kernel Module

On a Linux system with headers matching its running kernel:

```bash
cd part2
make
sudo insmod src/my_timer.ko
cat /proc/timer
sleep 2
cat /proc/timer
sudo rmmod my_timer
```

The first read shows the current time. Later reads also show the elapsed time since the last read. Removing the module removes `/proc/timer`.

## Part 3: Custom Kernel Setup

**Part 3 cannot run on an unmodified stock kernel.** The custom kernel must be configured, compiled, installed, and booted before the elevator module can be loaded.

The development setup uses Linux 6.16.12. The required changes to that kernel source tree are:

1. Copy `part3/syscalls.c` into the kernel's `syscalls/syscalls.c` file.
2. Create a `syscalls/Makefile` containing `obj-y += syscalls.o`.
3. Register the three calls in `arch/x86/entry/syscalls/syscall_64.tbl`:
   - `548 common start_elevator sys_start_elevator`
   - `549 common issue_request sys_issue_request`
   - `550 common stop_elevator sys_stop_elevator`
4. Add matching declarations for all three functions to `include/linux/syscalls.h`.
5. Add the `syscalls/` directory to the kernel Makefile's `core-y` directories.
6. Configure and build the modified kernel, install the kernel and modules, reboot, and check `uname -r` to confirm the custom kernel is running.

The numbered entries above describe the custom syscall registration used for this project. The kernel changes and a matching build configuration must be reproduced before trying Part 3. The complete install/boot process has not yet been verified, so this section should be expanded with the exact successful commands after that test.

## Part 3: Elevator Module

After booting the **modified** kernel, from the repository root:

```bash
cd part3
make
sudo insmod src/elevator.ko
cat /proc/elevator
```

The course-provided programs can then be used to control and test the elevator:

```bash
./consumer --start
./producer 10
watch -n 1 cat /proc/elevator
./consumer --stop
```

The `producer` and `consumer` executables shown here are course-provided test programs; they are not currently included in the file listing above. Run these commands only after obtaining and compiling those programs as instructed by the course.

Once the elevator has stopped and it is safe to unload:

```bash
sudo rmmod elevator
```

## Cleaning Build Files

To remove generated files for one part, run `make clean` inside that part's directory. From the project root, `make clean` runs the clean targets for all three parts. Cleaning kernel modules requires the matching kernel build directory to be available.

Compiled executables, `.o` files, and `.ko` files should not be committed to GitHub.

---

# Development Log

**EXAMPLE ENTRIES — REPLACE BEFORE SUBMISSION.** The dates and individual work notes below are sample wording based on the original division of labor. They are **not verified records** of work performed by the named members. Replace them with each person's real dates and contributions. The October 13 division-of-labor submission and November 2 local development/tests are the dates that have been established.

### Vivianna Loredo

| Date | Work Completed / Notes (Example) |
| --- | --- |
| 2026-10-13 | Reviewed the project instructions and the sections assigned to me, including Part 1, the timer module, linked lists, mutexes, and the scheduling algorithm. The division-of-labor document was submitted on Canvas. |
| 2026-10-18 | Worked on the system-call tracing programs and compared the trace files to check that Part 1 made exactly five additional calls. |
| 2026-10-23 | Reviewed the timer module, especially how `/proc/timer` prints the current time and calculates the elapsed time after another read. |
| 2026-10-28 | Worked on the linked-list and mutex portions of the elevator and checked how pet requests should be stored and protected. |
| 2026-11-02 | Helped review the elevator's scheduling rules, checked the Part 1 and Part 2 test results, and worked on the final README and repository organization. |

### Damian Abrego

| Date | Work Completed / Notes (Example) |
| --- | --- |
| 2026-10-13 | Reviewed the division of labor and the assigned kernel-related sections, including system calls, kernel compilation, threads, linked lists, mutexes, and elevator scheduling. |
| 2026-10-18 | Worked through the required system-call registration changes and how the wrappers connect to the elevator module. |
| 2026-10-23 | Reviewed the modified kernel configuration and compilation process and checked the files needed to register the elevator calls. |
| 2026-10-28 | Worked on the kernel thread and linked-list behavior, including moving between floors and keeping track of waiting and onboard pets. |
| 2026-11-02 | Reviewed mutex synchronization and scheduling behavior and checked the remaining kernel integration work before submission. |

### Xavier Rosario

| Date | Work Completed / Notes (Example) |
| --- | --- |
| 2026-10-13 | Reviewed the project requirements and the assigned sections, including Part 1, Part 2, adding system calls, kernel compilation, and the elevator thread. |
| 2026-10-18 | Worked on the two Part 1 programs and used `strace` to compare the baseline program with the five additional calls. |
| 2026-10-23 | Worked on the timer module and checked the current-time output, elapsed-time calculation, and cleanup when unloading. |
| 2026-10-28 | Reviewed the custom kernel configuration and system-call wrappers and checked how they would be used by the elevator module. |
| 2026-11-02 | Helped review the elevator thread and compilation status and checked the remaining testing steps needed for the demonstration. |

---

# Meetings

**EXAMPLE MEETING LOG — REPLACE BEFORE SUBMISSION.** These are examples of how to document group meetings, not a record that the meetings occurred. Update the dates, attendees, format, and discussion to match actual meetings.

| Example Date | Example Attendees | Format | Discussion / Outcome (Example) |
| --- | --- | --- | --- |
| 2026-10-13 | Vivianna Loredo, Xavier Rosario, Damian Abrego | [Confirm format] | Reviewed the project requirements and went through the division of labor. Discussed which tasks depended on the modified kernel and which ones could be tested separately. |
| 2026-10-22 | Vivianna Loredo, Xavier Rosario, Damian Abrego | [Confirm format] | Discussed progress on Part 1 and the timer module and checked the steps required to add the three new system calls. |
| 2026-10-29 | Vivianna Loredo, Xavier Rosario, Damian Abrego | [Confirm format] | Reviewed the elevator thread, linked lists, mutexes, scheduling requirements, and remaining kernel build tasks. |
| 2026-11-02 | Vivianna Loredo, Xavier Rosario, Damian Abrego | [Confirm format] | Discussed the current compilation and testing status and what still needed to be finished before the demonstration. |

---

# Testing

The following results have been checked in the development environment:

## Part 1

- Both programs compiled successfully.
- `empty.trace` had 30 traced system calls.
- `part1.trace` had 35 traced system calls.
- The second program made exactly five additional calls.

## Part 2

- `my_timer.ko` built successfully against the running Ubuntu 6.8 kernel.
- Loading the module created `/proc/timer`.
- The first read printed the current time.
- Reads after `sleep 2` and `sleep 3` printed elapsed times of approximately 2.04 and 3.03 seconds.
- Unloading the module removed `/proc/timer`.

## Part 3

- The elevator C source compiled into `elevator.o` against the running Ubuntu 6.8 headers as a preliminary C compilation check.
- A separate Linux 6.16.12 kernel build was started with the custom system-call additions.
- The updated system-call wrappers and elevator source still need a full build and integration test on the custom kernel.
- The course-provided producer/consumer tests and the final demonstration have not yet been run.

---

# Bugs / Known Issues

No issues were reported in the completed Part 1 and Part 2 tests above. Part 3 is **not yet fully validated**: the custom kernel must finish compiling and boot successfully, and the elevator still needs its complete loading, scheduling, stopping, and unloading tests. This section should be updated with any issues found during those tests.

---

# Considerations

The project is organized into separate folders because each part has different compilation and testing requirements. Parts 1 and 2 can be checked on a regular Linux installation with the appropriate tools and kernel headers. Part 3 needs a compatible x86-64 Linux environment running the custom kernel with the added system calls.

The GitHub repository is intended to contain the C source, Makefiles, trace files, and documentation rather than compiled binaries or modules. Because the kernel is not included in the repository, anyone testing Part 3 will need to follow the custom-kernel setup instructions first. Before the final demonstration, we still need to confirm that the elevator module can be built, loaded, tested, and removed safely on that kernel.

**Repository status:** [Confirm the GitHub URL and successful upload after pushing the project files.]
