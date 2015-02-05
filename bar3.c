char *bar3(void * _printk, int a, char * c) {
  void (*ptr)(const char *fmt, ...);
  ptr = _printk;
  ptr("The attacking code just called printk! : "
      "With parameters passing \n");
  ptr(c);
  ptr("%d\n", a);

  return "Hello World!";
}
