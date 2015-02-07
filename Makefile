obj-m += test_driver.o
obj-m += test_driver2.o

#obj-m += bar4_tot.o
#bar4_tot-y := bar4.o

default: all foo.so bar.so bar2.so bar3.so test ioctl bar4.so

test: test.c
	gcc $< -o $@ -ldl

ioctl: ioctl.c
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
#	gcc -I. -o $@ -c $< -rdynamic

bar3.so: bar3.o
	gcc $< -o $@ -shared
	#gcc $< -o $@ -shared -rdynamic

bar4.o: bar4.c
	gcc -I /usr/include/ -o $@ -c $< -Wall -O2 -Wall -fPIC

#bar4.o: bar4.c
#	gcc -D__KERNEL__ -I /usr/src/linux-headers-$(shell uname -r)/include/ -o $@ -c $< -DMODULE -Wall -O2 -Wall -fPIC

#bar4.o: bar4.c
#	gcc -I. -o $@ -c $< -Wall -Werror -O3 -fPIC

bar4.so: bar4.o
	gcc $< -o $@ -shared

#	gcc -I. -I"/usr/src/linux-headers-$(shell uname -r)/include/" -o $@ -c $< -Wall -Werror -O3 -fPIC

foo.so: foo.o
	gcc -shared $< -o $@

foo.o: foo.c
	gcc -c -Wall -Werror -fpic $<

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
