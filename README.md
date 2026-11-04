# COP4610 Project 2 - Pet Elevator

**Group 36:** Vivianna Loredo, Xavier Rosario, Damian Abrego
**Language:** C
**Test environment:** Ubuntu Server 24.04 in VMware; custom Linux kernel `6.16.12-cop4610` for Part 3

## About the project

- **Part 1:** Use `strace` to compare an empty C program with one that makes five extra system calls.
- **Part 2:** Make a kernel module that creates `/proc/timer` and shows the current time and time since the last read.
- **Part 3:** Add three system calls and run a pet elevator inside the Linux kernel using a kernel thread, linked lists, mutexes, and dynamic memory allocation.

## Elevator requirements

- Five floors (1 through 5), with a maximum of **5 pets** and **50 lbs** onboard.
- Pets board in FIFO order on each floor. The elevator uses the first onboard pet's destination, or the oldest waiting request if empty, to choose where to go.
- Moving one floor takes **2 seconds**; loading or unloading takes **1 second** per stop.
- Pet types: Chihuahua (`0`, 3 lbs), Pug (`1`, 14 lbs), Pughuahua (`2`, 10 lbs), Dachshund (`3`, 16 lbs).
- Invalid floors, pet types, or requests going to the same floor return `1`. Valid requests return `0`.
- The elevator can finish delivering pets already onboard after a stop request.
- `/proc/elevator` shows its state, current floor, load, pets onboard, waiting pets, and number serviced.

| Syscall | Number | Purpose |
| --- | --- | --- |
| `start_elevator` | 548 | Start elevator |
| `issue_request` | 549 | Add a pet request |
| `stop_elevator` | 550 | Stop elevator |

## Division of labor

This was our original division. We also helped each other with debugging, testing, and getting the parts working together.

| Section | Assigned members |
| --- | --- |
| Part 1 - `strace` | Vivianna, Xavier |
| Part 2 - Timer module | Vivianna, Xavier |
| Part 3A - System calls | Damian, Xavier |
| Part 3B - Kernel compilation | Damian, Xavier |
| Part 3C - Kernel thread | Damian, Xavier |
| Part 3D - Linked lists | Vivianna, Damian |
| Part 3E - Mutexes | Vivianna, Damian |
| Part 3F - Scheduling | Damian, Vivianna |

## Files

| Location | What it contains |
| --- | --- |
| `part1/` | `empty.c`, `part1.c`, trace files, Makefile |
| `part2/src/` | `my_timer.c` and module Makefile |
| `part3/syscalls.c` | Kernel system-call wrappers and pointers |
| `part3/src/elevator_main.c` | Elevator module setup and cleanup |
| `part3/src/pets.c` | Pet requests and linked lists |
| `part3/src/scheduler.c` | Elevator thread, movement, and loading |
| `part3/src/proc.c` | `/proc/elevator` output |
| `part3/src/elevator.h`, `elevator.c` | Shared definitions and earlier elevator source |
| `part3/tests/` | Provided producer, consumer, and system-call test programs |
| `Makefile`, `part*/Makefile` | Build commands for the project parts |

The actual elevator module is built from `elevator_main.c`, `pets.c`, `scheduler.c`, and `proc.c` through `part3/src/Makefile`.

## How to run

Requires Ubuntu/Linux, GCC, Make, `strace`, kernel build dependencies, and `sudo`. Kernel modules must be built for the running kernel.

**Part 1**

```bash
cd part1
make
make trace
```

**Part 2**

```bash
cd part2
make
sudo insmod src/my_timer.ko
cat /proc/timer
sleep 2 && cat /proc/timer
sudo rmmod my_timer
```

**Part 3**

We modified and installed Linux `6.16.12-cop4610` by adding `part3/syscalls.c` to the kernel's `syscalls/` directory, registering calls 548-550 in `arch/x86/entry/syscalls/syscall_64.tbl`, declaring them in `include/linux/syscalls.h`, and including `syscalls/` in the kernel Makefile. The custom kernel must be installed and booted **before** running Part 3. Check with `uname -r`.

```bash
cd part3
make
sudo insmod elevator.ko
cd tests/elevator-test
make
./consumer --start
./producer 10
watch -n 1 cat /proc/elevator
```

Press Ctrl+C to close `watch`, then:

```bash
./consumer --stop
sudo rmmod elevator
```

`producer` sometimes generates floor 6; those requests are rejected because the building only has five floors. Don't load the separate `syscheck.ko` test module at the same time as `elevator.ko`.

## Testing

We tested Parts 1-3 on our Ubuntu VM on November 4, 2026.

| Test | Result |
| --- | --- |
| Part 1: `strace` | 30 calls in `empty.trace`, 35 in `part1.trace`; all 5 added calls verified |
| Part 2: `/proc/timer` | Compiled and loaded; elapsed times around 2.002s and 3.002s; proc entry removed on unload |
| Custom kernel | `6.16.12-cop4610` compiled, installed, booted, and accessible over SSH |
| Elevator module | Built and loaded; `/proc/elevator` worked; unload/reload and counter reset worked |
| Start/stop | Normal start, stop, restart, duplicate start, and stopping with a pet onboard passed |
| Invalid inputs | Invalid floors, same-floor requests, and invalid pet types rejected |
| Weight limit | Four pets boarded at 50 lbs; fifth waited; all 5 delivered |
| Pet limit | Five pets boarded; sixth waited; all 6 delivered |
| FIFO and multiple floors | Same-floor FIFO boarding order verified; multi-floor requests completed |
| Movement timing | About 2s between floors and 1s for loading/unloading in a controlled test |
| Stress test | 36 requests over about 3 minutes: 26 accepted, 10 rejected, all 26 accepted pets delivered |

The stress test ended with 0 pets onboard, 0 waiting, and the elevator back to `IDLE`. These tests cover the main behavior, but not every possible concurrent case.

## Development log

| Date | Member | Work |
| --- | --- | --- |
| Nov 2 | Vivianna | Worked on the initial project files, elevator logic, and fixing the loading time. |
| Nov 4 | Vivianna | Got the custom kernel running, tested all three parts, and fixed syscall synchronization. |
| Nov 2-4 | Xavier | Worked on splitting up and integrating the elevator code, system calls, and test programs. |
| Nov 2-4 | Xavier | Helped debug issues, test elevator behavior, and check that the parts worked together. |
| Nov 2-4 | Damian | Worked on the kernel/elevator implementation, scheduling, and integrating project parts. |
| Nov 2-4 | Damian | Helped with testing, debugging, checking requirements, and updating documentation. |

## Group meetings

| Date | Members | Format | Topics |
| --- | --- | --- | --- |
| Nov 2 | Vivianna, Xavier, Damian | Zoom / group chat | Project progress, code integration, kernel compilation, and testing |

## Demo note

Our working custom kernel is installed in the Ubuntu VMware VM on the Windows PC. The GitHub repository contains the source code, not the installed kernel. To demonstrate Part 3 on a different machine, we need access to that VM or another machine with the patched kernel installed.
