/*  
 *  test_driver.c - To simulate a malicious kernel module which directly executes
 *                  code read from a .so file via vfs_read() while initialization.
 */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */

#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include<linux/slab.h>

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/vmalloc.h>

#if 0
unsigned long **sys_call_table;
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);

static unsigned long **aquire_sys_call_table(void)
{
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;

  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) 
      return sct;

    offset += sizeof(void *);
  }
  
  return NULL;
}
#endif

static char* (*bar)(void);
static char* s;

struct file* file_open(const char* path, int flags, int rights) {
  struct file* filp = NULL;
  mm_segment_t oldfs;
  int err = 0;

  oldfs = get_fs();
  set_fs(get_ds());
  filp = filp_open(path, flags, rights);
  set_fs(oldfs);
  if(IS_ERR(filp)) {
      err = PTR_ERR(filp);
      return NULL;
  }
  return filp;
}

void file_close(struct file* file) {
  filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, 
    unsigned char* data, unsigned int size) {
  mm_segment_t oldfs;
  int ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_read(file, data, size, &offset);

  set_fs(oldfs);
  return ret;
}

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
