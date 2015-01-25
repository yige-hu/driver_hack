/*  
 *  test_driver2.c - To simulate a malicious kernel module which executes
 *                   machine code written by a process via ioctl.
 */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/fs.h>
#include <asm/uaccess.h>  /* for get_user and put_user */

#if 0
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include<linux/slab.h>
#endif

#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/vmalloc.h>

#include "test_driver2.h"
#define SUCCESS 0
#define DEVICE_NAME "test_driver2"
#define BUF_LEN 7491

static char* (*bar)(void);
static char* s;

#ifndef DEBUG
#define DEBUG 1
#endif

static int attacked = 0;

/* 
 * Is the device open right now? Used to prevent
 * concurent access into the same device 
 */
static int Device_Open = 0;

/* 
 * The message the device will give when asked 
 */
static char Message[BUF_LEN];

/* 
 * How far did the process reading the message get?
 * Useful if the message is larger than the size of the
 * buffer we get to fill in device_read. 
 */
static char *Message_Ptr;

/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
  printk(KERN_INFO "device_open(%p)\n", file);
#endif

  /* 
   * We don't want to talk to two processes at the same time 
   */
  if (Device_Open)
    return -EBUSY;

  Device_Open++;
  /*
   * Initialize the message 
   */
  Message_Ptr = Message;
  try_module_get(THIS_MODULE);
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
  printk(KERN_INFO "device_release(%p,%p)\n", inode, file);
#endif

  /* 
   * We're now ready for our next caller 
   */
  Device_Open--;

  module_put(THIS_MODULE);
  return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file, /* see include/linux/fs.h   */
         char __user * buffer,  /* buffer to be
               * filled with data */
         size_t length, /* length of the buffer     */
         loff_t * offset)
{
  /* 
   * Number of bytes actually written to the buffer 
   */
  int bytes_read = 0;

#ifdef DEBUG
  printk(KERN_INFO "device_read(%p,%p,%d)\n", file, buffer, (int) length);
#endif

  /* 
   * If we're at the end of the message, return 0
   * (which signifies end of file) 
   */
  if (*Message_Ptr == 0)
    return 0;

  /* 
   * Actually put the data into the buffer 
   */
  while (length && *Message_Ptr) {

    /* 
     * Because the buffer is in the user data segment,
     * not the kernel data segment, assignment wouldn't
     * work. Instead, we have to use put_user which
     * copies data from the kernel data segment to the
     * user data segment. 
     */
    put_user(*(Message_Ptr++), buffer++);

    length--;
    bytes_read++;
  }

#ifdef DEBUG
  printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, (int) length);
#endif

  /* 
   * Read functions are supposed to return the number
   * of bytes actually inserted into the buffer 
   */
  return bytes_read;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 */
static ssize_t
device_write(struct file *file,
       const char __user * buffer, size_t length, loff_t * offset)
{
  int i;

  char *buf;
  int size = BUF_LEN;
  buf = __vmalloc(size + 1, GFP_KERNEL, PAGE_KERNEL_EXEC);

#ifdef DEBUG
  printk(KERN_INFO "device_write(%p,%s,%d)", file, buffer, (int) length);
#endif

  for (i = 0; i < length && i < BUF_LEN; i++) {
    get_user(Message[i], buffer + i);

    buf[i] = Message[i];
  }

  Message_Ptr = Message;


  /* Let's do the hack here! */

  if (!(attacked ++)) {
    printk(KERN_INFO "Starts the attack....\n");

    /* Execute the attacking code by function pointer. */
    bar = (void *) (buf + 0x0000000000000680);
    s = bar();
    printk(KERN_INFO "result: %s\n", s);
  }


  /* 
   * Again, return the number of input characters used 
   */
  return i;
}

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
long device_ioctl(struct file *file, /* ditto */
     unsigned int ioctl_num,  /* number and param for ioctl */
     unsigned long ioctl_param)
{
  int i;
  char *temp;
  char ch;

  /* 
   * Switch according to the ioctl called 
   */
  switch (ioctl_num) {
  case IOCTL_SET_MSG:
    /* 
     * Receive a pointer to a message (in user space) and set that
     * to be the device's message.  Get the parameter given to 
     * ioctl by the process. 
     */
    temp = (char *)ioctl_param;

    /* 
     * Find the length of the message 
     */
    get_user(ch, temp);
    for (i = 0; ch && i < BUF_LEN; i++, temp++)
      get_user(ch, temp);

    device_write(file, (char *)ioctl_param, i, 0);
    break;

  case IOCTL_GET_MSG:
    /* 
     * Give the current message to the calling process - 
     * the parameter we got is a pointer, fill it. 
     */
    i = device_read(file, (char *)ioctl_param, 99, 0);

    /* 
     * Put a zero at the end of the buffer, so it will be 
     * properly terminated 
     */
    put_user('\0', (char *)ioctl_param + i);
    break;

  case IOCTL_GET_NTH_BYTE:
    /* 
     * This ioctl is both input (ioctl_param) and 
     * output (the return value of this function) 
     */
    return Message[ioctl_param];
    break;
  }

  return SUCCESS;
}

/* Module Declarations */

/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations Fops = {
  .read = device_read,
  .write = device_write,
  .unlocked_ioctl = device_ioctl,
  .open = device_open,
  .release = device_release,  /* a.k.a. close */
};

/* 
 * Initialize the module - Register the character device 
 */
int init_module()
{
  int ret_val;
  /* 
   * Register the character device (atleast try) 
   */
  ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

  /* 
   * Negative values signify an error 
   */
  if (ret_val < 0) {
    printk(KERN_ALERT "%s failed with %d\n",
           "Sorry, registering the character device ", ret_val);
    return ret_val;
  }

  printk(KERN_INFO "%s The major device number is %d.\n",
         "Registeration is a success", MAJOR_NUM);
  printk(KERN_INFO "If you want to talk to the device driver,\n");
  printk(KERN_INFO "you'll have to create a device file. \n");
  printk(KERN_INFO "We suggest you use:\n");
  printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
  printk(KERN_INFO "The device file name is important, because\n");
  printk(KERN_INFO "the ioctl program assumes that's the\n");
  printk(KERN_INFO "file you'll use.\n");

  return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
//  int ret;

  /* 
   * Unregister the device 
   */
  unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

  /* 
   * If there's an error, report it 
   */
//  if (ret < 0)
//    printk(KERN_ALERT "Error: unregister_chrdev: %d\n", ret);
}





//////////////////////////////////////////////////////////////////

#if 0

int init_module(void) {
//  mm_segment_t fs;
  struct file* fd;
  char *buf;
  int size;
  int flags;
  mode_t mode;
  int ret;
//  int prot;

  printk(KERN_INFO "Starts loading the attacked driver....\n");

  /* Read from the shared library. */
#if 0
  if(!(sys_call_table = aquire_sys_call_table()))
    return -1;

  ref_sys_read = (void *)sys_call_table[__NR_read];  
#endif

//  fs = get_fs();
//  set_fs(KERNEL_DS);
  fd = (struct file *) kmalloc(sizeof(struct file), GFP_KERNEL);
//  buf = kmalloc(7846, GFP_KERNEL);
  size = 7941;
  buf = __vmalloc(size + 1, GFP_KERNEL, PAGE_KERNEL_EXEC);
  flags = O_LARGEFILE | O_RDONLY | __FMODE_EXEC | MAY_READ | MAY_EXEC | MAY_OPEN;
  mode = 0640;

#if 0
  fd = sys_open("foo.so", flags, mode);
  if(fd != -1) {
    sys_read(fd, buf, size);
    sys_close(fd);
  }
  set_fs(fs);
#endif

  fd = file_open("bar.so", flags, mode);
  if(fd != NULL) {
    if ((ret = file_read(fd, 0, buf, size)) <= 0) {
      printk(KERN_INFO "File reading failed. ret = %d\n", ret);
      return -1;
    }
    file_close(fd);

    printk(KERN_INFO "File readed successfully. Execution starts....\n");

    /* Execute the attacking code by function pointer. */
    bar = (void *) (buf + 0x0000000000000680);
    s = bar();
    printk(KERN_INFO "result: %s\n", s);
  }

  return 0;
}

void cleanup_module(void)
{
  printk(KERN_INFO "The attacked driver is removed.\n");
}

#endif
