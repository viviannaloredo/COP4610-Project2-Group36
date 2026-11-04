obj-m += syscheck.o

all: syscheck.ko test-syscalls

run:
	./test-syscalls

test-syscalls: test-syscalls.c test-syscalls.h
	gcc -Wall -Wextra test-syscalls.c -o test-syscalls

syscheck.ko: syscheck.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

.PHONY: all clean run

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f test-syscalls
