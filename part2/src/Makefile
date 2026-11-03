KDIR := /lib/modules/$(shell uname -r)/build

# Build in src/, then place my_timer.ko in this directory
all:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src modules
	cp src/my_timer.ko .

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src clean
	rm -f my_timer.ko

.PHONY: all clean
