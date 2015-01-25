#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

long sc_page_size;

void* hlib;
char* (*bar)(void * printf, int a, char * c);

char* s;

int main(int argc, char** argv) {

#if 0
  if ( !(hlib = dlopen("./bar.so", RTLD_LAZY)) ) {
    printf("Cannot load bar.so\n");
    return 1;
  }
  if ( !(bar = dlsym(hlib, "bar")) ) {
    printf("Cannot read symbol bar\n");
    return 2;
  }

#else 

  int size = 7847;
  char *addr = NULL;
  int fd = open("bar2.so", O_RDONLY);

#if 0
  char *buf = malloc(7941 + 1);
  read(fd, buf, size);
  close(fd);
#endif

  addr = mmap(NULL, size + 1,
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE, fd, 0);

//  sc_page_size = sysconf(_SC_PAGE_SIZE);
//#define PAGE_ALIGN(addr) \
//    ({ (long int) addr & ~(sc_page_size - 1); })
//  mprotect((unsigned char *) PAGE_ALIGN(buf), size + 1, 
//      PROT_READ | PROT_WRITE | PROT_EXEC);

  perror("mmap");

  bar = (void *) addr + 0x0000000000000680;
#endif

  s = bar(printf, 0, "Test parameter passing\n");
  perror("");
  printf("result: %s\n", s);      
  return 0;
} 
