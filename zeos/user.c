#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    
  while(1) { 
    int i = gettime();
    for (int j = 0; j < 10000000; j++) __asm__ __volatile__ ("");
    int j = gettime();
    //Utilitzo aquesta linia per fer saltar una excepcio en cas de que j <= i
    //Per comprovar que gettime funciona be
    if (j - i <= 0) __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0)); 
  }
}
