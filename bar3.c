char *bar3(void * printk, int a, char * c) {
  void (*ptr)(const char *fmt, ...);
  ptr = printk;
  ptr("The attacking code just called printk!"
      "With parameters passing \n");
  ptr(c);

  return "Hello World!";
}
