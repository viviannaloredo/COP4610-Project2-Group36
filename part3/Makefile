KDIR := /lib/modules/$(shell uname -r)/build

# Build in src/, then place elevator.ko in this directory
all:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src modules
	cp src/elevator.ko .

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src clean
	rm -f elevator.ko

.PHONY: all clean
