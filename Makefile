obj-m += test_driver.o

default: all foo.so bar.so test

test: test.c
	gcc $< -o $@ -ldl

bar.o: bar.c
	gcc -I. -o bar.o -c bar.c -Wall -Werror -O3 -fPIC

bar.so: bar.o
	gcc -o bar.so bar.o -shared

foo.so: foo.o
	gcc -shared $< -o $@

foo.o: foo.c
	gcc -c -Wall -Werror -fpic $<

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
