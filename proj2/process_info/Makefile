obj-m := process_info_module.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	g++ -o test phase2test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm test process_info_module.o.ur-safe
