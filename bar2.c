char *bar2(void * printf, int a, char * c) {
  void (*ptr)(const char *, ...);
  ptr = printf;
  ptr(c);

  return "Hello, World!";
}
