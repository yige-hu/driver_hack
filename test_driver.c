/*  
 *  test_driver.c - The simplest kernel module.
 */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */

int init_module(void)
{
  printk(KERN_INFO "Starts loading the attacked driver....\n");

  /* Read from the shared library. */
  

  /* Execute the attacking code by function pointer. */


  return 0;
}

void cleanup_module(void)
{
  printk(KERN_INFO "The attacked driver is removed.\n");
}
