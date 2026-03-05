#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{

  
  int i = gettime();
  for (int j = 0; j < 1000000; j++) asm ("");
  int j = gettime();

  int n = write(1, "Hola mundo\n", 11);
  if (n < 0) {
    perror("write");
  }

  n = write(2, "Esto debe fallar\n", 18);
  if (n < 0) {
    perror("write");
  }

  //Utilitzo aquesta linia per fer saltar una excepcio en cas de que j <= i
  //Per comprovar que gettime funciona be
  //if (j <= i) __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0)); 

  while(1);
}
