obj-m += test_driver.o
obj-m += test_driver2.o

default: all bar3.so ioctl

ioctl: ioctl.c
	gcc $< -o $@ -ldl

bar3.o: bar3.c
	gcc -I. -o $@ -c $< -Wall -Werror -O3 -fPIC

bar3.so: bar3.o
	gcc $< -o $@ -shared

foo.so: foo.o
	gcc -shared $< -o $@

foo.o: foo.c
	gcc -c -Wall -Werror -fpic $<

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
