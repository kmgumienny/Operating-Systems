obj-m := open_close_intercept.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	g++ -o test phase1test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm test open_close_intercept.o.ur-safe
