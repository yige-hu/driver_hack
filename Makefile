obj-m += test_driver.o
obj-m += test_driver2.o

default: all foo.so bar.so bar2.so bar3.so test

test: test.c
	gcc $< -o $@ -ldl

bar.o: bar.c
	gcc -I. -o $@ -c $< -Wall -Werror -O3 -fPIC

bar.so: bar.o
	gcc $< -o $@ -shared

bar2.o: bar2.c
	gcc -I. -o $@ -c $< -Wall -Werror -O3 -fPIC

bar2.so: bar2.o
	gcc $< -o $@ -shared

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
