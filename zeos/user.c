#include <libc.h>

char buff[24];

int pid;

//Testing function to test the GDB connection and the clock interrupt
void testing_gdb() {
  int i;
  for (i = 0; i < 1000; ++i) asm("nop"); 
} 

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    
  while(1) { 
    testing_gdb();
  }
}
