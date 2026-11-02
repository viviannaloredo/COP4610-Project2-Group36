KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR)/src clean

.PHONY: all clean
