#include <stdio.h>
#include <dlfcn.h>

void* hlib;
char* (*bar)(void);

char* s;

int main(int argc, char** argv) {
  if ( !(hlib = dlopen("./lib.so", RTLD_LAZY)) ) {
    printf("Cannot load lib.so\n");
    return 1;
  }
  if ( !(bar = dlsym(hlib, "bar")) ) {
    printf("Cannot read symbol bar\n");
    return 2;
  }
  s = bar();
  printf("result: %s\n", s);      
  return 0;
} 
