#include <linux/linkage.h>
//#include "printk.h"
#include <linux/kernel.h>

extern int printk(const char *fmt, ...);

char *bar4(void * _printk, int a, char * c) {
  void (*ptr)(const char *fmt, ...);
  ptr = _printk;
  ptr("The attacking code just called printk! : "
      "With parameters passing \n");
  ptr(c);
  ptr("%d\n", a);

//  void *addr_ptrk = (void *) (0x27e1a049);
//  void (*ptr2) (const char *fmt, ...);
//  ptr2 = addr_ptrk;
  ptr("Now test printk:\n");
  printk("Try to call printk without passed as a funtion pointer\n");
  printk("%d\n", a);

  return "Hello World!";
}
