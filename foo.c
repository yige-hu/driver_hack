void foo(void * _printk, int a, char * c) {
  void (*ptr)(const char *, ...);
  ptr = _printk;
  ptr(c);
}
