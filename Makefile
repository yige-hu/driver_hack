obj-m += test_driver.o

default: all foo.so

test: test.c
	gcc $< -o $@ -ldl

lib.o: lib.c
	gcc -I. -o lib.o -c lib.c -Wall -Werror -O3 -fPIC

lib.so: lib.o
	gcc -o lib.so lib.o -shared

foo.so: foo.o
	gcc -shared $< -o $@

foo.o: foo.c
	gcc -c -Wall -Werror -fpic $<

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
