obj-m += misc-module.o

KDIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

all: module app

module:
	make -C $(KDIR) M=$(PWD) modules

app:
	gcc -o test-app test-app.c

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f test-app
