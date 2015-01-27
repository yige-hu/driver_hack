This is a tiny testing driver to demonstrate that a malicious driver is able to 
read from a shared library (.so) file and execute pieces of functions in it
via calling it from the buffer.

Currently please only use "test_driver2", which interact with a user-level 
process to get the attacking code in .so file.
"test_driver.c" attemps to ask the driver to read the attacking code itself
via vfs_read() while initializtion. But I'm still debugging it. (I won't be
debugging it. But it's conceptually ok.)

Usage:

1. Compilation:
  >$ Make

2. You might need to alter some parameters if it's compiled on a different
   plantform as mine, since they are hard-coded:
   In "test_driver2.h", BUF_LEN is the file size of bar3.so;
   In "test_driver2.c", 0x0000000000000680 is the position of the symbol "bar3"
       in bar3.so (can be read by "objdump -Fd").

3. Install the module and register the correspondent device file:
  >$ sudo su

  >$ insmod ./test_driver2.ko

  >$ mknod test_driver2 c 100 0

4. Let the driver to execute the attacking code. Currently, my program only
   supports doing it one-time (it should be easy to allow multiple calls):
  (a) via device_read():
  >$ cat bar3.so > driver_dev

  (b) or via ioctl():
  >$ ./ioctl

5. To see the execution effect of the attacking code:
  >$ dmesg | tail
